/*  Computation Thread Pool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This is a thread pool for compute-heavy tasks.
 *
 *  This thread pool has a limited number of threads and should only be used for
 *  compute-heavy tasks that do not block or yield. A blocked thread will still
 *  count towards the thread limit.
 *
 *  Because the # of threads is capped, it is safe to spam this thread pool with
 *  lots of smaller tasks.
 *
 */

#ifndef PokemonAutomation_ComputationThreadPoolCore_H
#define PokemonAutomation_ComputationThreadPoolCore_H

#include <functional>
//#include <list>
#include <deque>
#include <thread>
#include "Common/Cpp/CpuUtilization/CpuUtilization.h"
#include "Common/Cpp/Stopwatch.h"
#include "AsyncTask.h"

namespace PokemonAutomation{




class ComputationThreadPoolCore final{
public:
    ComputationThreadPoolCore(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads
    );
    ~ComputationThreadPoolCore();

    size_t current_threads() const{
        std::lock_guard<std::mutex> lg(m_lock);
        return m_threads.size();
    }
    size_t max_threads() const{
        return m_max_threads;
    }
    WallDuration cpu_time() const;

    void ensure_threads(size_t threads);
//    void wait_for_everything();


public:
    //  As of this writing, tasks dispatched earlier are not allowed to block
    //  on tasks that are dispatched later as it may cause a deadlock.

    //  Dispatch the function. If there are no threads available, it waits until
    //  there are.
    [[nodiscard]] std::unique_ptr<AsyncTask> blocking_dispatch(std::function<void()>&& func);

    //  Dispatch the function. Returns null if no threads are available.
    //  "func" will be moved-from only on success.
    [[nodiscard]] std::unique_ptr<AsyncTask> try_dispatch(std::function<void()>& func);

    //  Run function for all the indices [start, end).
    //  Lower indices are not allowed to block on higher indices.
    void run_in_parallel(
        const std::function<void(size_t index)>& func,
        size_t start, size_t end,
        size_t block_size = 0
    );


private:
    struct ThreadData{
        std::thread thread;
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
    std::deque<AsyncTask*> m_queue;

    std::deque<ThreadData> m_threads;

    bool m_stopping;
    size_t m_busy_count;
    mutable std::mutex m_lock;
    std::condition_variable m_thread_cv;
    std::condition_variable m_dispatch_cv;
};




}
#endif
