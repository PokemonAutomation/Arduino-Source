/*  Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnitTestRunner_H
#define PokemonAutomation_UnitTestRunner_H

#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "UnitTest.h"

namespace PokemonAutomation{



class UnitTestRunner final : public CancellableScope{
public:
    ~UnitTestRunner();
    UnitTestRunner(
        Logger& logger,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    )
        : UnitTestRunner(nullptr, logger, max_memory, max_threads)
    {}
    UnitTestRunner(
        CancellableScope& parent,
        Logger& logger,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    )
        : UnitTestRunner(&parent, logger, max_memory, max_threads)
    {}
    UnitTestRunner(
        CancellableScope* parent,
        Logger& logger,
        uint64_t max_memory = 0,
        size_t max_threads = 0
    );

    template <typename TestType, class... Args>
    void add_test(Args&&... args){
        add_test(std::make_unique<TestType>(std::forward<Args>(args)...));
    }
    void add_test(std::unique_ptr<UnitTest> test);

    void wait_for_all();

    virtual bool cancel(std::exception_ptr reason) noexcept override;


private:
    void thread_loop();

    void dispatch_test(const std::string& name);


private:
    Logger& m_logger;
    uint64_t m_max_memory = 0;
    size_t m_max_threads = 1;

    Mutex m_lock;
    ConditionVariable m_cv;

    uint64_t m_current_memory = 0;
    size_t m_current_threads = 0;

    struct PendingEntry{
        std::unique_ptr<UnitTest> test;
        std::multimap<uint64_t, std::string>::iterator iter_total;
        std::multimap<double, std::string>::iterator iter_density;
    };
    struct DispatchedEntry{
        std::unique_ptr<UnitTest> test;
        AsyncTask task;
    };

    std::map<std::string, DispatchedEntry> m_completed;
    std::map<std::string, DispatchedEntry> m_currently_running;
    std::map<std::string, PendingEntry> m_test_by_name;
    std::multimap<uint64_t, std::string> m_test_by_total_memory;
    std::multimap<double, std::string> m_test_by_per_thread_memory;
    AsyncTask m_dispatcher;
};




}
#endif
