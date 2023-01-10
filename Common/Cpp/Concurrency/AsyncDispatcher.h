/*  Async Task Runner
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      This class is meant for asynchronous tasks, not for parallel computation.
 * This class will always spawn enough threads run all tasks in parallel.
 *
 * If you need to spam a bunch of compute tasks in parallel, use ParallelTaskRunner.
 *
 */

#ifndef PokemonAutomation_AsyncDispatcher_H
#define PokemonAutomation_AsyncDispatcher_H

#include <vector>
#include <deque>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace PokemonAutomation{

class AsyncTask{
public:
    //  Wait for the task to finish before destructing. Doesn't rethrow exceptions.
    ~AsyncTask();

    //  If the task ended with an exception, rethrow it here.
    //  This does not clear the exception.
    void rethrow_exceptions();

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions();


private:
    template <class... Args>
    AsyncTask(Args&&... args)
        : m_task(std::forward<Args>(args)...)
        , m_finished(false)
        , m_stopped_with_error(false)
    {}
    void signal();

private:
    friend class FireForgetDispatcher;
    friend class AsyncDispatcher;
    friend class ParallelTaskRunner;

    std::function<void()> m_task;
    bool m_finished;
    std::atomic<bool> m_stopped_with_error;
    std::exception_ptr m_exception;
    std::mutex m_lock;
    std::condition_variable m_cv;
};

class AsyncDispatcher{
public:
//    AsyncDispatcher(size_t starting_threads);
    AsyncDispatcher(std::function<void()>&& new_thread_callback, size_t starting_threads);
    ~AsyncDispatcher();

    //  Ensure a certain # of threads so they don't need to be lazily created.
    void ensure_threads(size_t threads);

    //  Dispatch the specified task and return a handle to it.
    //  Call "handle->wait()" to wait for the task to finish.
    std::unique_ptr<AsyncTask> dispatch(std::function<void()>&& func);

    //  Run the specified lambda for indices [s, e) in parallel.
    void run_in_parallel(
        size_t s, size_t e,
        const std::function<void(size_t index)>& func
    );

private:
    void dispatch_task(AsyncTask& task);
    void thread_loop();

private:
    std::function<void()> m_new_thread_callback;
    std::deque<AsyncTask*> m_queue;
    std::vector<std::thread> m_threads;
    bool m_stopping;
    size_t m_busy_count;
    std::mutex m_lock;
    std::condition_variable m_cv;
};




}

#endif
