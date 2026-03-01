/*  Thread Pool (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <thread>
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "AsyncTask_Default.h"
#include "ThreadPool_Default.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ThreadPool_Default::ThreadPool_Default(
    std::function<void()>&& new_thread_callback,
    size_t starting_threads,
    size_t max_threads
)
    : m_new_thread_callback(std::move(new_thread_callback))
    , m_max_threads(max_threads == 0 ? std::thread::hardware_concurrency() : max_threads)
    , m_stopping(false)
    , m_busy_count(0)
{
    for (size_t c = 0; c < starting_threads; c++){
        spawn_thread();
    }
}
ThreadPool_Default::~ThreadPool_Default(){
    stop();
}
void ThreadPool_Default::stop(){
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (m_stopping) return;
        m_stopping = true;
        m_thread_cv.notify_all();
//        m_dispatch_cv.notify_all();
    }
    for (ThreadData& thread : m_threads){
        if (thread.thread.joinable()){
            thread.thread.join();
        }
    }

    // DO NOT JOIN AGAIN IN DESTRUCTOR
    m_threads.clear();

    for (auto& task : m_queue){
        task->report_cancelled();
    }

    // DO NOT CLEAR AGAIN IN DESTRUCTOR
    m_queue.clear();
}


void ThreadPool_Default::ensure_threads(size_t threads){
    std::lock_guard<Mutex> lg(m_lock);
    while (m_threads.size() < threads){
        spawn_thread();
    }
}
#if 0
void ThreadPoolCore::wait_for_everything(){
    std::unique_lock<Mutex> lg(m_lock);
    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count == 0;
    });
}
#endif




WallDuration ThreadPool_Default::cpu_time() const{
    //  TODO: Don't lock the entire queue.
    WallDuration ret = WallDuration::zero();
    std::lock_guard<Mutex> lg(m_lock);
    for (const ThreadData& thread : m_threads){
//        ret += thread_cpu_time(thread.handle);
        ret += thread.runtime.total();
    }
    return ret;
}

AsyncTask ThreadPool_Default::dispatch(std::function<void()>&& func){
    AsyncTask task;
    {
        std::unique_lock<Mutex> lg(m_lock);
        AsyncTaskCore*& ptr = m_queue.emplace_back();
        try{
            task = AsyncTask(std::make_unique<AsyncTask_Cpp>(std::move(func)));
            ptr = task.core();
            ptr->report_started();
        }catch (...){
            m_queue.pop_back();
            throw;
        }
    }
    m_thread_cv.notify_one();
    return task;
}
AsyncTask ThreadPool_Default::dispatch_now_blocking(std::function<void()>&& func){
    AsyncTask task;
    {
        std::unique_lock<Mutex> lg(m_lock);

        m_dispatch_cv.wait(lg, [this]{
            return m_queue.size() + m_busy_count < m_max_threads;
        });

        //  Enqueue task.
        AsyncTaskCore*& ptr = m_queue.emplace_back();
        try{
            spawn_threads();
            task = AsyncTask(std::make_unique<AsyncTask_Cpp>(std::move(func)));
            ptr = task.core();
            ptr->report_started();
        }catch (...){
            m_queue.pop_back();
            throw;
        }
    }
    m_thread_cv.notify_one();
    return task;
}
AsyncTask ThreadPool_Default::try_dispatch_now(std::function<void()>& func){
    AsyncTask task;
    {
        std::lock_guard<Mutex> lg(m_lock);

        if (m_queue.size() + m_busy_count >= m_max_threads){
            return AsyncTask();
        }

        //  Enqueue task.
        AsyncTaskCore*& ptr = m_queue.emplace_back();
        try{
            spawn_threads();
            task = AsyncTask(std::make_unique<AsyncTask_Cpp>(std::move(func)));
            ptr = task.core();
            ptr->report_started();
        }catch (...){
            m_queue.pop_back();
            throw;
        }
    }
    m_thread_cv.notify_one();
    return task;
}


void ThreadPool_Default::run_in_parallel(
    const std::function<void(size_t index)>& func,
    size_t start, size_t end,
    size_t block_size
){
    if (start >= end){
        return;
    }
    size_t total = end - start;

    if (block_size == 0){
        block_size = total / m_max_threads / 16;
        if (block_size == 0){
            block_size = 1;
        }
    }

    size_t blocks = (total + block_size - 1) / block_size;

    //  Prepare all the tasks.
    std::vector<AsyncTask> tasks;
    for (size_t c = 0; c < blocks; c++){
        tasks.emplace_back(
            std::make_unique<AsyncTask_Cpp>([=, &func]{
                size_t s = start + c * block_size;
                size_t e = std::min(s + block_size, end);
//                cout << "Running: [" << s << "," << e << ")" << endl;
                for (; s < e; s++){
                    func(s);
                }
            })
        );
    }

    {
        //  Enqueue all the tasks.
        std::unique_lock<Mutex> lg(m_lock);
        for (AsyncTask& task : tasks){
            m_queue.emplace_back(task.core())->report_started();
            m_thread_cv.notify_one();
        }
        spawn_threads();

        //  Use this thread to process the queue until our tasks are done.
        while (!m_queue.empty() && !tasks.back().is_finished()){
            AsyncTaskCore* task = m_queue.front();
            m_queue.pop_front();

            ReverseLockGuard<Mutex> lg0(m_lock);
            task->run();
        }
    }

    //  Wait for everything to finish.
    for (AsyncTask& task : tasks){
        task.wait_and_rethrow_exceptions();
    }
}



void ThreadPool_Default::spawn_thread(){
    //  Must call under lock.
    ThreadData& handle = m_threads.emplace_back();
    try{
        handle.thread = Thread([&, this]{
            run_with_catch(
                "ParallelTaskRunner::thread_loop()",
                [&, this]{ thread_loop(handle); }
            );
        });
    }catch (...){
        m_threads.pop_back();
        throw;
    }
}
void ThreadPool_Default::spawn_threads(){
    while (m_threads.size() < std::min(m_queue.size() + m_busy_count, m_max_threads)){
        spawn_thread();
    }
}
void ThreadPool_Default::thread_loop(ThreadData& data){
    data.handle = current_thread_handle();

    if (m_new_thread_callback){
        m_new_thread_callback();
    }

    data.runtime.start();

    std::unique_lock<Mutex> lg(m_lock);
    m_busy_count++;
    while (!m_stopping){
//        cout << "m_queue... " << m_queue.size() << endl;
        if (m_queue.empty()){
            data.runtime.stop();
            m_busy_count--;
            m_dispatch_cv.notify_all();
//            cout << "waiting... " << m_busy_count << endl;
            m_thread_cv.wait(lg);
//            cout << "waking... " << m_busy_count << endl;
            m_busy_count++;
            data.runtime.start();
            continue;
        }

        AsyncTaskCore* task = m_queue.front();
        m_queue.pop_front();

        ReverseLockGuard<Mutex> lg0(m_lock);
        task->run();
    }
}




}
