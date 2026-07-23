/*  Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/MemoryUtilization/MemoryUtilization.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "UnitTestRunner.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




UnitTestRunner::~UnitTestRunner(){
    detach();
    UnitTestRunner::cancel(nullptr);
    m_dispatcher.wait_and_ignore_exceptions();


    std::unique_lock<Mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        return m_currently_running.empty();
    });
}
UnitTestRunner::UnitTestRunner(
    CancellableScope* parent,
    Logger& logger,
    uint64_t max_memory,
    size_t max_threads
)
    : m_logger(logger)
    , m_max_memory(max_memory)
    , m_max_threads(max_threads)
{
    if (m_max_memory == 0){
        MemoryUsage memory = process_memory_usage();
        m_max_memory = memory.total_system_memory - memory.total_used_system_memory;
    }
    if (m_max_threads == 0){
        m_max_threads = std::thread::hardware_concurrency() * 2;
    }
    m_dispatcher = GlobalThreadPools::unlimited_normal().dispatch_now_blocking([this]{
        thread_loop();
    });
    if (parent){
        attach(*parent);
    }
}


void UnitTestRunner::add_test(std::unique_ptr<UnitTest> test){
    throw_if_cancelled();

    if (test->name().empty()){
        InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Test name cannot be null.");
    }

    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_test_by_name.contains(test->name())){
            InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Duplicate Test Name: " + test->name());
        }
        auto iter0 = m_test_by_name.end();
        auto iter1 = m_test_by_total_memory.end();
        auto iter2 = m_test_by_per_thread_memory.end();
        try{
            iter0 = m_test_by_name.emplace(test->name(), PendingEntry{}).first;
            iter1 = m_test_by_total_memory.emplace(test->memory(), test->name());
            iter2 = m_test_by_per_thread_memory.emplace((double)test->memory() / test->threads(), test->name());
            iter0->second.test = std::move(test);
            iter0->second.iter_total = iter1;
            iter0->second.iter_density = iter2;
        }catch (...){
            if (iter0 != m_test_by_name.end()){
                m_test_by_name.erase(iter0);
            }
            if (iter1 != m_test_by_total_memory.end()){
                m_test_by_total_memory.erase(iter1);
            }
            if (iter2 != m_test_by_per_thread_memory.end()){
                m_test_by_per_thread_memory.erase(iter2);
            }
            throw;
        }
    }

    m_cv.notify_all();
}

void UnitTestRunner::wait_for_all(){
    std::unique_lock<Mutex> lg(m_lock);
    while (!cancelled()){
        if (m_currently_running.empty() && m_test_by_name.empty()){
            return;
        }
        m_cv.wait(lg);
    }
}

bool UnitTestRunner::cancel(std::exception_ptr reason) noexcept{
    if (CancellableScope::cancel(reason)){
        return true;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
    }
    m_cv.notify_all();
    return false;
}


void UnitTestRunner::thread_loop(){
    m_logger.log(
        "Starting UnitTestRunner with:"
        "\n    Max Memory: " + tostr_bytes(m_max_memory) +
        "\n    Max Threads: " + tostr_u_commas(m_max_threads)
    );
//    cout << "thread_loop() - start" << endl;
    std::unique_lock<Mutex> lg(m_lock);
    while (!cancelled()){
        if (m_test_by_name.empty()){
            m_cv.wait(lg);
            continue;
        }

        //  If nothing is running, always run the thing that uses the most memory.
        if (m_currently_running.empty()){
            dispatch_test(m_test_by_total_memory.rbegin()->second);
            continue;
        }

        //  We are over the resource limit. Don't run anything.
        if (m_current_memory >= m_max_memory || m_current_threads >= m_max_threads){
            m_cv.wait(lg);
            continue;
        }

        //  Try to run the test that uses the most memory.
        {
            const std::string& name = m_test_by_total_memory.rbegin()->second;
            PendingEntry& entry = m_test_by_name.find(name)->second;
            if (m_current_memory + entry.test->memory() <= m_max_memory ||
                m_current_threads + entry.test->threads() <= m_max_threads
            ){
                dispatch_test(name);
                continue;
            }
        }

        //  Try run the smallest memory/thread ratio.
        {
            const std::string& name = m_test_by_per_thread_memory.begin()->second;
            PendingEntry& entry = m_test_by_name.find(name)->second;
            if (m_current_memory + entry.test->memory() <= m_max_memory ||
                m_current_threads + entry.test->threads() <= m_max_threads
            ){
                dispatch_test(name);
                continue;
            }
        }

        //  Can't run anything.
        m_cv.wait(lg);
    }
//    cout << "thread_loop() - end" << endl;
}


void UnitTestRunner::dispatch_test(const std::string& name){
    //  Must be called under the lock.

//    cout << "dispatch_test() - start" << endl;
    auto iter = m_test_by_name.find(name);
    PendingEntry& entry = iter->second;

    auto iter_current = m_currently_running.emplace(name, std::move(entry.test)).first;
    DispatchedEntry& node = iter_current->second;

    UnitTest& test = *iter_current->second.test;
    m_current_memory += test.m_memory;
    m_current_threads += test.m_threads;

    {
        ReverseLockGuard<Mutex> lg(m_lock);
        node.task = GlobalThreadPools::computation_normal().dispatch_now_blocking([this, iter_current]{
            UnitTest& test = *iter_current->second.test;
            test.m_result = UnitTestResult::FAILED;
            try{
                m_logger.log("Starting: " + test.name());
                auto ret = test.run(*this);
                test.m_result = ret.first;
                test.m_message = std::move(ret.second);
            }catch (Exception& e){
                test.m_message = e.to_str();
            }catch (std::bad_alloc&){
                test.m_result = UnitTestResult::OOM;
            }catch (std::exception& e){
                test.m_message = e.what();
            }catch (...){
                test.m_message = "Unknown exception.";
            }

            switch (test.result()){
            case UnitTestResult::NOT_RUN:
            case UnitTestResult::PASSED:
                m_logger.log("Passed: " + test.name(), COLOR_BLUE);
                break;
            case UnitTestResult::FAILED:
                m_logger.log("Failed: " + test.name() + ", Message: " + test.message(), COLOR_RED);
                break;
            case UnitTestResult::SKIPPED:
                m_logger.log("Skipped: " + test.name(), COLOR_ORANGE);
                break;
            case UnitTestResult::OOM:
                m_logger.log("Out-of-Memory: " + test.name(), COLOR_ORANGE);
                break;
            }

            {
                std::lock_guard<Mutex> lg(m_lock);
                auto node = m_currently_running.extract(iter_current);
                m_completed.insert(std::move(node));
                m_current_memory -= test.m_memory;
                m_current_threads -= test.m_threads;
            }
            m_cv.notify_all();
        });
    }

    m_test_by_total_memory.erase(entry.iter_total);
    m_test_by_per_thread_memory.erase(entry.iter_density);
    m_test_by_name.erase(iter);
//    cout << "dispatch_test() - end" << endl;
}




}
