/*  Computation Thread Pool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PanicDump.h"
#include "ReverseLockGuard.h"
#include "ComputationThreadPoolCore.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ComputationThreadPoolCore::ComputationThreadPoolCore(
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
ComputationThreadPoolCore::~ComputationThreadPoolCore(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_thread_cv.notify_all();
//        m_dispatch_cv.notify_all();
    }
    for (ThreadData& thread : m_threads){
        thread.thread.join();
    }
    for (auto& task : m_queue){
        task->report_cancelled();
    }
}

WallDuration ComputationThreadPoolCore::cpu_time() const{
    //  TODO: Don't lock the entire queue.
    WallDuration ret = WallDuration::zero();
    std::lock_guard<std::mutex> lg(m_lock);
    for (const ThreadData& thread : m_threads){
//        ret += thread_cpu_time(thread.handle);
        ret += thread.runtime.total();
    }
    return ret;
}


void ComputationThreadPoolCore::ensure_threads(size_t threads){
    std::lock_guard<std::mutex> lg(m_lock);
    while (m_threads.size() < threads){
        spawn_thread();
    }
}
#if 0
void ComputationThreadPoolCore::wait_for_everything(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count == 0;
    });
}
#endif

std::unique_ptr<AsyncTask> ComputationThreadPoolCore::blocking_dispatch(std::function<void()>&& func){
    std::unique_ptr<AsyncTask> task(new AsyncTask(std::move(func)));

    {
        std::unique_lock<std::mutex> lg(m_lock);

        m_dispatch_cv.wait(lg, [this]{
            return m_queue.size() + m_busy_count < m_max_threads;
        });

        //  Enqueue task.
        m_queue.emplace_back(task.get())->report_started();
        spawn_threads();

#if 0
        //  Use this thread to process the queue until our task is
        while (!m_queue.empty() && !task->is_finished()){
            AsyncTask* current = m_queue.front();
            m_queue.pop_front();

            ReverseLockGuard<std::mutex> lg0(m_lock);
            current->run();
        }
#endif
    }

//    cout << "notify... " << endl;
    m_thread_cv.notify_one();

    return task;
}
std::unique_ptr<AsyncTask> ComputationThreadPoolCore::try_dispatch(std::function<void()>& func){
    std::unique_ptr<AsyncTask> task;
    {
        std::lock_guard<std::mutex> lg(m_lock);

        if (m_queue.size() + m_busy_count >= m_max_threads){
            return nullptr;
        }

        task.reset(new AsyncTask(std::move(func)));

        //  Enqueue task.
        m_queue.emplace_back(task.get())->report_started();

        spawn_threads();
    }

    m_thread_cv.notify_one();

    return task;
}


void ComputationThreadPoolCore::run_in_parallel(
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
    std::vector<std::unique_ptr<AsyncTask>> tasks(blocks);
    for (size_t c = 0; c < blocks; c++){
        tasks[c].reset(new AsyncTask([=, &func]{
            size_t s = start + c * block_size;
            size_t e = std::min(s + block_size, end);
//            cout << "Running: [" << s << "," << e << ")" << endl;
            for (; s < e; s++){
                func(s);
            }
        }));
    }

    {
        //  Enqueue all the tasks.
        std::unique_lock<std::mutex> lg(m_lock);
        for (std::unique_ptr<AsyncTask>& task : tasks){
            m_queue.emplace_back(task.get())->report_started();
            m_thread_cv.notify_one();
        }
        spawn_threads();

        //  Use this thread to process the queue until our tasks are done.
        while (!m_queue.empty() && !tasks.back()->is_finished()){
            AsyncTask* task = m_queue.front();
            m_queue.pop_front();

            ReverseLockGuard<std::mutex> lg0(m_lock);
            task->run();
        }
    }

    //  Wait for everything to finish.
    for (std::unique_ptr<AsyncTask>& task : tasks){
        task->wait_and_rethrow_exceptions();
    }
}



void ComputationThreadPoolCore::spawn_thread(){
    //  Must call under lock.
    ThreadData& handle = m_threads.emplace_back();
    try{
        handle.thread = std::thread(
            run_with_catch,
            "ParallelTaskRunner::thread_loop()",
            [&, this]{ thread_loop(handle); }
        );
    }catch (...){
        m_threads.pop_back();
        throw;
    }
}
void ComputationThreadPoolCore::spawn_threads(){
    while (m_threads.size() < std::min(m_queue.size() + m_busy_count, m_max_threads)){
        spawn_thread();
    }
}
void ComputationThreadPoolCore::thread_loop(ThreadData& data){
    data.handle = current_thread_handle();

    if (m_new_thread_callback){
        m_new_thread_callback();
    }

    data.runtime.start();

    std::unique_lock<std::mutex> lg(m_lock);
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

        AsyncTask* task = m_queue.front();
        m_queue.pop_front();

        ReverseLockGuard<std::mutex> lg0(m_lock);
        task->run();
    }
}




}
