/*  Thread Pool (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ThreadPoolCore_H
#define PokemonAutomation_ThreadPoolCore_H

#include <functional>
#include <deque>
#include "Common/Cpp/Stopwatch.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/Cpp/CpuUtilization/CpuUtilization.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"

namespace PokemonAutomation{




class ThreadPool_Default final : public ThreadPool{
public:
    ThreadPool_Default(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads = (size_t)-1
    );
    ~ThreadPool_Default();

    virtual void stop() override;
    virtual void ensure_threads(size_t threads) override;

//    void wait_for_everything();


public:
    virtual size_t current_threads() const override{
        std::lock_guard<Mutex> lg(m_lock);
        return m_threads.size();
    }
    virtual size_t max_threads() const override{
        return m_max_threads;
    }
    virtual WallDuration cpu_time() const override;


public:
    [[nodiscard]] virtual AsyncTask dispatch(std::function<void()>&& func) override;
    [[nodiscard]] virtual AsyncTask dispatch_now_blocking(std::function<void()>&& func) override;
    [[nodiscard]] virtual AsyncTask try_dispatch_now(std::function<void()>& func) override;

    virtual void run_in_parallel(
        const std::function<void(size_t index)>& func,
        size_t start, size_t end,
        size_t block_size = 0
    ) override;


private:
    struct ThreadData{
        Thread thread;
        ThreadHandle handle;
        Stopwatch runtime;
    };

    void spawn_thread();
    void spawn_threads();
    void thread_loop(ThreadData& data);


private:
    std::function<void()> m_new_thread_callback;
    size_t m_max_threads;
    std::deque<AsyncTaskCore*> m_queue;

    std::deque<ThreadData> m_threads;

    bool m_stopping;
    size_t m_busy_count;
    mutable Mutex m_lock;
    ConditionVariable m_thread_cv;
    ConditionVariable m_dispatch_cv;
};




}
#endif
