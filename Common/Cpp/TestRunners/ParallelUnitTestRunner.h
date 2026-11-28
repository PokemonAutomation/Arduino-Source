/*  Parallel Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ParallelUnitTestRunner_H
#define PokemonAutomation_ParallelUnitTestRunner_H

#include <map>
#include <atomic>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "UnitTest.h"

namespace PokemonAutomation{



//  Run unit tests in parallel, within a memory and a thread limit.
class UnitTestRunner final : public CancellableScope{
public:
    static constexpr size_t MAX_PASSED_TESTS_TO_LOG = 100;

    struct Listener{
        virtual void on_test_finished(
            std::shared_ptr<const UnitTest> test,
            UnitTestResult result
        ) = 0;
    };
    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    ~UnitTestRunner();
    UnitTestRunner(
        Logger& logger,
        ThreadPool& thread_pool,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    )
        : UnitTestRunner(nullptr, logger, thread_pool, max_memory, max_threads)
    {}
    UnitTestRunner(
        CancellableScope& parent,
        Logger& logger,
        ThreadPool& thread_pool,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    )
        : UnitTestRunner(&parent, logger, thread_pool, max_memory, max_threads)
    {}
    UnitTestRunner(
        CancellableScope* parent,
        Logger& logger,
        ThreadPool& thread_pool,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    );

    template <typename TestType, class... Args>
    void add_test(Args&&... args){
        add_test(std::make_shared<const TestType>(std::forward<Args>(args)...));
    }
    void add_test(std::shared_ptr<const UnitTest> test);


public:
    void run();

    virtual bool cancel(std::exception_ptr reason) noexcept override;


private:
    void dispatch_test(const std::string& name);

    //  Log the result of a test that just finished: a passed test at Info, a skipped
    //  test at Warning, and a failed or out-of-memory test at Error.
    //  Called by the test's task in `dispatch_test()`, from the thread pool.
    void log_test_result(const UnitTest& test, const UnitTestResult& result);


private:
    Logger& m_logger;
    ThreadPool& m_thread_pool;

    //  # of tests added by `add_test()`
    std::atomic<size_t> m_total_tests = 0;
    //  # of tests finished so far.
    std::atomic<size_t> m_finished_tests = 0;

    uint64_t m_max_memory = 0;
    size_t m_max_threads = 1;

    Mutex m_lock;
    ConditionVariable m_cv;

    uint64_t m_current_memory = 0;
    size_t m_current_threads = 0;

    struct PendingEntry{
        std::shared_ptr<const UnitTest> test;
        std::multimap<uint64_t, std::string>::iterator iter_total;
        std::multimap<double, std::string>::iterator iter_density;
    };
    struct DispatchedEntry{
        std::shared_ptr<const UnitTest> test;
        AsyncTask task;
    };

    std::map<std::string, DispatchedEntry> m_completed;
    std::map<std::string, DispatchedEntry> m_currently_running;
    std::map<std::string, PendingEntry> m_test_by_name;
    std::multimap<uint64_t, std::string> m_test_by_total_memory;
    std::multimap<double, std::string> m_test_by_per_thread_memory;

    ListenerSet<Listener> m_listeners;
};




}
#endif
