/*  Thread Pool
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "PanicDump.h"
#include "AsyncDispatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


AsyncTask::~AsyncTask(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [=]{ return m_finished; });
}
void AsyncTask::rethrow_exceptions(){
    if (!m_stopped_with_error.load(std::memory_order_acquire)){
        return;
    }
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_exception){
        std::rethrow_exception(m_exception);
    }
}
void AsyncTask::wait_and_rethrow_exceptions(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [=]{ return m_finished; });
#if 0
    if (m_exception && std::uncaught_exceptions() == 0){
        std::rethrow_exception(m_exception);
    }
#else
    if (m_exception){
        std::rethrow_exception(m_exception);
    }
#endif
}
void AsyncTask::signal(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_finished = true;
    m_cv.notify_all();
}


#if 0
AsyncDispatcher::AsyncDispatcher(size_t starting_threads)
    : AsyncDispatcher(nullptr, starting_threads)
{}
#endif
AsyncDispatcher::AsyncDispatcher(std::function<void()>&& new_thread_callback, size_t starting_threads)
    : m_new_thread_callback(std::move(new_thread_callback))
    , m_stopping(false)
    , m_busy_count(0)
{
    for (size_t c = 0; c < starting_threads; c++){
        m_threads.emplace_back(run_with_catch, "AsyncDispatcher::thread_loop()", [=]{ thread_loop(); });
    }
}
AsyncDispatcher::~AsyncDispatcher(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    for (std::thread& thread : m_threads){
        thread.join();
    }
    for (AsyncTask* task : m_queue){
        task->signal();
    }
}
void AsyncDispatcher::dispatch_task(AsyncTask& task){
    cout << "dispatch_task() - enter" << endl;
    std::lock_guard<std::mutex> lg(m_lock);

    //  Enqueue task.
    m_queue.emplace_back(&task);

    //  Make sure a thread is ready for it.
    if (m_queue.size() > m_threads.size() - m_busy_count){
        m_threads.emplace_back(run_with_catch, "AsyncDispatcher::thread_loop()", [=]{ thread_loop(); });
    }

    m_cv.notify_one();
    cout << "dispatch_task() - exit" << endl;
}

std::unique_ptr<AsyncTask> AsyncDispatcher::dispatch(std::function<void()>&& func){
    std::unique_ptr<AsyncTask> task(new AsyncTask(std::move(func)));
    dispatch_task(*task);
    cout << "dispatch_task - 1() - exit" << endl;
    return task;
}
void AsyncDispatcher::run_in_parallel(
    size_t s, size_t e,
    const std::function<void(size_t index)>& func
){
    if (s >= e){
        return;
    }

    //  Build tasks.
    std::vector<std::unique_ptr<AsyncTask>> tasks;
    for (size_t index = 0; s + index < e; index++){
        size_t local_index = s + index;
        tasks.emplace_back(new AsyncTask(
            [&func, local_index]{ func(local_index); }
        ));
    }

    {
        std::lock_guard<std::mutex> lg(m_lock);

        //  Enqueue tasks.
        for (std::unique_ptr<AsyncTask>& task : tasks){
            m_queue.emplace_back(task.get());
        }

        //  Make sure there are enough threads.
        while (m_queue.size() > m_threads.size() - m_busy_count){
            m_threads.emplace_back(run_with_catch, "AsyncDispatcher::thread_loop()", [=]{ thread_loop(); });
        }

        for (size_t c = 0; c < tasks.size(); c++){
            m_cv.notify_one();
        }
    }

    //  Wait
//    cout << "begin wait()" << endl;
    for (std::unique_ptr<AsyncTask>& task : tasks){
        task->wait_and_rethrow_exceptions();
    }
//    cout << "end wait()" << endl;
}



void AsyncDispatcher::thread_loop(){
    if (m_new_thread_callback){
        m_new_thread_callback();
    }
    bool busy = false;
    while (true){
        AsyncTask* task;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (busy){
                m_busy_count--;
                busy = false;
            }

            if (m_stopping){
                return;
            }
            if (m_queue.empty()){
                m_cv.wait(lg);
                continue;
            }

            task = m_queue.front();
            m_queue.pop_front();

            busy = true;
            m_busy_count++;
        }

        try{
            task->m_task();
        }catch (...){
            task->m_exception = std::current_exception();
            task->m_stopped_with_error.store(true, std::memory_order_release);
//            cout << "Task threw an exception." << endl;
//            std::lock_guard<std::mutex> lg(m_lock);
//            for (AsyncTask* t : m_queue){
//                t->signal();
//            }
        }
        task->signal();
    }
}





}

