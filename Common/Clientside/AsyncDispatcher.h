/*  Async Task Runner
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_AsyncDispatcher_H
#define PokemonAutomation_AsyncDispatcher_H

#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

class AsyncDispatcher;

class AsyncTask{
public:
    //  Wait for the task to finish before destructing. Doesn't rethrow exceptions.
    ~AsyncTask();

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait();

private:
    template <class... Args>
    AsyncTask(Args&&... args)
        : m_task(std::forward<Args>(args)...)
        , m_finished(false)
    {}

    void signal();

private:
    friend class AsyncDispatcher;
    std::function<void()> m_task;
    bool m_finished;
    std::exception_ptr m_exception;
    std::mutex m_lock;
    std::condition_variable m_cv;
};

class AsyncDispatcher{
public:
    AsyncDispatcher(size_t starting_threads = 0);
    ~AsyncDispatcher();

#if 0
    //  Dispatch the specified task and return a handle to it.
    //  Call "handle->wait()" to wait for the task to finish.
    template <class... Args>
    std::unique_ptr<AsyncTask> dispatch(Args&&... args){
        std::unique_ptr<AsyncTask> task(new AsyncTask(std::forward<Args>(args)...));
        dispatch_task(task.get());
        return task;
    }
#endif

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
    std::deque<AsyncTask*> m_queue;
    std::vector<std::thread> m_threads;
    bool m_stopping;
    size_t m_busy_count;
    std::mutex m_lock;
    std::condition_variable m_cv;
};



}

#endif
