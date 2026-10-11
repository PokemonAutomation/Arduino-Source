/*  Parallel Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ParallelUnitTestRunner_H
#define PokemonAutomation_ParallelUnitTestRunner_H

#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "UnitTest.h"

namespace PokemonAutomation{



class UnitTestRunner final : public CancellableScope{
public:
    struct Listener{
        //  Called after each run of a test.
        //  If test image distortions is enabled, a test can run several times,
        //  once per distortion. During this call, `test->distortion_label()` is the
        //  label of the run that finished, so use `test->name_with_distortion()` to name it.
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

    // Add a unit test instance.
    template <typename TestType, class... Args>
    void add_test(Args&&... args){
        add_test(std::make_shared<const TestType>(std::forward<Args>(args)...));
    }
    // Add a unit test instance.
    void add_test(std::shared_ptr<const UnitTest> test);

    //  Initialize the image distortion augmentation on each supported image unit test.
    //
    //  After called, it will load distortion resources from the test folder. During `run()`,
    //  it will check if a test has color distortion augmentation available and run the
    //  test with different distortions it after the basic unit test is run.
    //
    //  Throws if the calibration data can't be loaded.
    void enable_test_image_distortions(const std::string& test_repo_path);


public:
    // Main function to run the all tests added by `add_test()`.
    void run();

    virtual bool cancel(std::exception_ptr reason) noexcept override;


private:
    void dispatch_test(const std::string& name);

    //  Run `test` once with its current distortion label, log the result and return it.
    //  Exceptions thrown by the test are turned into a failed (or out-of-memory) result.
    UnitTestResult run_test(const UnitTest& test);


private:
    Logger& m_logger;
    ThreadPool& m_thread_pool;

    bool m_test_image_distortions = false;

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
