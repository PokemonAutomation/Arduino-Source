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
