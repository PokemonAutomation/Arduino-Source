/*  Parallel Task Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if _WIN32
#include <Windows.h>
#endif
#include "Common/Cpp/PanicDump.h"
#include "ParallelTaskRunner.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ParallelTaskRunner::ParallelTaskRunner(
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
        m_threads.emplace_back(run_with_catch, "ParallelTaskRunner::thread_loop()", [this]{ thread_loop(); });
    }
}
ParallelTaskRunner::~ParallelTaskRunner(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_thread_cv.notify_all();
//        m_dispatch_cv.notify_all();
    }
    for (std::thread& thread : m_threads){
        thread.join();
    }
    for (auto& task : m_queue){
        task->signal();
    }
}

void ParallelTaskRunner::wait_for_everything(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count == 0;
    });
}

std::shared_ptr<AsyncTask> ParallelTaskRunner::dispatch(std::function<void()>&& func){
    std::shared_ptr<AsyncTask> task(new AsyncTask(std::move(func)));

    std::unique_lock<std::mutex> lg(m_lock);

    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count < m_max_threads;
    });

    //  Enqueue task.
    m_queue.emplace_back(task);

    if (m_queue.size() + m_busy_count > m_threads.size()){
        m_threads.emplace_back(run_with_catch, "ParallelTaskRunner::thread_loop()", [this]{ thread_loop(); });
    }

    m_thread_cv.notify_one();

    return task;
}


void ParallelTaskRunner::run_in_parallel(
    const std::function<void(size_t index)>& func,
    size_t start, size_t end,
    size_t block_size
){
    if (start >= end){
        return;
    }
    size_t total = end - start;
    size_t blocks = (total + block_size - 1) / block_size;

    std::vector<std::shared_ptr<AsyncTask>> tasks;
    for (size_t c = 0; c < blocks; c++){
        tasks.emplace_back(dispatch([=, &func]{
            size_t s = start + c * block_size;
            size_t e = std::min(s + block_size, end);
//            cout << "Running: [" << s << "," << e << ")" << endl;
            for (; s < e; s++){
                func(s);
            }
        }));
    }

    for (std::shared_ptr<AsyncTask>& task : tasks){
        task->wait_and_rethrow_exceptions();
    }
}




void ParallelTaskRunner::thread_loop(){
//#if _WIN32
//    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
//#endif
    if (m_new_thread_callback){
        m_new_thread_callback();
    }

    bool busy = false;
    while (true){
        std::shared_ptr<AsyncTask> task;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (busy){
                m_busy_count--;
                busy = false;
                m_dispatch_cv.notify_all();
            }

            if (m_stopping){
                return;
            }
            if (m_queue.empty()){
                m_thread_cv.wait(lg);
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
//            std::lock_guard<std::mutex> lg(m_lock);
//            for (std::shared_ptr<AsyncTask>& t : m_queue){
//                t->signal();
//            }
        }
        task->signal();
    }
}



}
