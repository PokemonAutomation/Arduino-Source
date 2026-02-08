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




class ThreadPool_Default final : public ThreadPoolCore{
public:
    ThreadPool_Default(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads
    );
    ~ThreadPool_Default();

    size_t current_threads() const{
        std::lock_guard<Mutex> lg(m_lock);
        return m_threads.size();
    }
    size_t max_threads() const{
        return m_max_threads;
    }
    WallDuration cpu_time() const;

    void ensure_threads(size_t threads);

    void stop();
//    void wait_for_everything();


public:
    //  As of this writing, tasks dispatched earlier are not allowed to block
    //  on tasks that are dispatched later as it may cause a deadlock.

    //  Dispatch the function. If there are no threads available, it waits until
    //  there are.
    [[nodiscard]] AsyncTask blocking_dispatch(std::function<void()>&& func);

    //  Dispatch the function. Returns null if no threads are available.
    //  "func" will be moved-from only on success.
    [[nodiscard]] AsyncTask try_dispatch(std::function<void()>& func);

    //  Run function for all the indices [start, end).
    //  Lower indices are not allowed to block on higher indices.
    void run_in_parallel(
        const std::function<void(size_t index)>& func,
        size_t start, size_t end,
        size_t block_size = 0
    );


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
    struct Data;

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
