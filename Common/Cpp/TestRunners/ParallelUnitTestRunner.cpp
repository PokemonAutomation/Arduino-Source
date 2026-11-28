/*  Parallel Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/MemoryUtilization/MemoryUtilization.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "ParallelUnitTestRunner.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

const std::string& UNIT_TEST_DEBUG_PATH(){
    static std::string path = DEBUG_PATH() + "UnitTests/";
    return path;
}


UnitTestRunner::~UnitTestRunner(){
    detach();
}
UnitTestRunner::UnitTestRunner(
    CancellableScope* parent,
    Logger& logger,
    ThreadPool& thread_pool,
    uint64_t max_memory,
    size_t max_threads
)
    : m_logger(logger)
    , m_thread_pool(thread_pool)
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
    if (parent){
        attach(*parent);
    }
}


void UnitTestRunner::add_test(std::shared_ptr<const UnitTest> test){
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
        m_total_tests.fetch_add(1, std::memory_order_relaxed);
    }

    m_cv.notify_all();
}


//  Return how often a passed test is logged: once every this many finished tests.
//  It is `ceil(total_tests / MAX_PASSED_TESTS_TO_LOG)`, so no more than
//  `MAX_PASSED_TESTS_TO_LOG` passed tests are logged, and it is at least 1.
static size_t passed_test_log_interval(size_t total_tests){
    size_t interval = (total_tests + UnitTestRunner::MAX_PASSED_TESTS_TO_LOG - 1) / UnitTestRunner::MAX_PASSED_TESTS_TO_LOG;
    return interval == 0 ? 1 : interval;
}

void UnitTestRunner::run(){
    const size_t total_tests = m_total_tests.load(std::memory_order_relaxed);
    const size_t interval = passed_test_log_interval(total_tests);
    m_logger.log(
        "Starting UnitTestRunner with:"
        "\n    Tests: " + tostr_u_commas(total_tests) +
        "\n    Max Memory: " + tostr_bytes(m_max_memory) +
        "\n    Max Threads: " + tostr_u_commas(m_max_threads) +
        "\n    Passed Tests Logged: " + (interval == 1
            ? std::string("all")
            : "1 in every " + tostr_u_commas(interval) + " finished tests"
        ) +
        "\n    Failed/Skipped Tests Logged: all"
    );
//    cout << "thread_loop() - start" << endl;
    std::unique_lock<Mutex> lg(m_lock);
    while (!cancelled()){
        if (m_test_by_name.empty()){
            break;
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
            if (m_current_memory + entry.test->memory() <= m_max_memory &&
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
            if (m_current_memory + entry.test->memory() <= m_max_memory &&
                m_current_threads + entry.test->threads() <= m_max_threads
            ){
                dispatch_test(name);
                continue;
            }
        }

        //  Can't run anything.
        m_cv.wait(lg);
    }

    //  Wait for everything running to finish.
    m_cv.wait(lg, [this]{
        if (!m_currently_running.empty()){
            return false;
        }
        if (!cancelled() && !m_test_by_name.empty()){
            return false;
        }
        return true;
    });
    m_completed.clear();
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




//  Run one test and return its result. An exception thrown by the test becomes its result:
//  FAILED with the exception's message, or OOM for std::bad_alloc.
//  Called by the task `dispatch_test()` starts for each test.
static UnitTestResult run_and_catch(const UnitTest& test, Logger& logger, CancellableScope& scope){
    try{
        return test.run(logger, scope);
    }catch (Exception& e){
        return UnitTestResult(UnitTestResult::FAILED, e.to_str());
    }catch (std::bad_alloc&){
        return UnitTestResult::OOM;
    }catch (std::exception& e){
        return UnitTestResult(UnitTestResult::FAILED, e.what());
    }catch (...){
        return UnitTestResult(UnitTestResult::FAILED, "Unknown exception.");
    }
}


void UnitTestRunner::dispatch_test(const std::string& name){
    //  Must be called under the lock.

//    cout << "dispatch_test() - start" << endl;
    auto iter = m_test_by_name.find(name);
    PendingEntry& entry = iter->second;

    auto iter_current = m_currently_running.emplace(name, std::move(entry.test)).first;
    DispatchedEntry& node = iter_current->second;

    const UnitTest& test = *iter_current->second.test;
    m_current_memory += test.memory();
    m_current_threads += test.threads();

    {
        ReverseLockGuard<Mutex> lg(m_lock);
        node.task = m_thread_pool.dispatch_now_blocking([this, iter_current]{
            const UnitTest& test = *iter_current->second.test;
            UnitTestResult result = run_and_catch(test, m_logger, *this);

            log_test_result(test, result);

            m_listeners.run_method(
                &Listener::on_test_finished,
                std::move(iter_current->second.test),
                std::move(result)
            );

            {
                std::lock_guard<Mutex> lg(m_lock);
                auto node = m_currently_running.extract(iter_current);
                m_completed.insert(std::move(node));
                m_current_memory -= test.memory();
                m_current_threads -= test.threads();
            }
            m_cv.notify_all();
        });
    }

    m_test_by_total_memory.erase(entry.iter_total);
    m_test_by_per_thread_memory.erase(entry.iter_density);
    m_test_by_name.erase(iter);
//    cout << "dispatch_test() - end" << endl;
}


void UnitTestRunner::log_test_result(const UnitTest& test, const UnitTestResult& result){
    const size_t finished = m_finished_tests.fetch_add(1, std::memory_order_relaxed) + 1;
    const size_t total = m_total_tests.load(std::memory_order_relaxed);
    const std::string prefix = "[" + std::to_string(finished) + "/" + std::to_string(total) + "] ";

    switch (result.result){
    case UnitTestResult::PASSED:
        if (finished % passed_test_log_interval(total) == 0 || finished == total){
            m_logger.log(prefix + "PASSED: " + test.name(), COLOR_BLUE);
        }
        break;
    case UnitTestResult::FAILED:
        m_logger.log(prefix + "FAILED: " + test.name() + ", Message: " + result.message, COLOR_RED, LogLevel::Error);
        break;
    case UnitTestResult::SKIPPED:
        m_logger.log(prefix + "SKIPPED: " + test.name(), COLOR_ORANGE, LogLevel::Warning);
        break;
    case UnitTestResult::OOM:
        m_logger.log(prefix + "OOM: " + test.name(), COLOR_ORANGE, LogLevel::Error);
        break;
    }
}




}
