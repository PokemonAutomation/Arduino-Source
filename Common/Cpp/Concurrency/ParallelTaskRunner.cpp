/*  Parallel Task Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#if _WIN32
#include <Windows.h>
#endif
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/CpuUtilization/CpuUtilization.h"
#include "Common/Cpp/Stopwatch.h"
#include "ParallelTaskRunner.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{







class ParallelTaskRunnerCore final{
public:
    ParallelTaskRunnerCore(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads
    );
    ~ParallelTaskRunnerCore();

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

    //  Dispatch the function. If there are no threads available, it waits until
    //  there are.
    [[nodiscard]] std::unique_ptr<AsyncTask> blocking_dispatch(std::function<void()>&& func);

    //  Dispatch the function. Returns null if no threads are available.
    //  "func" will be moved-from only on success.
    [[nodiscard]] std::unique_ptr<AsyncTask> try_dispatch(std::function<void()>& func);

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







ParallelTaskRunner::ParallelTaskRunner(
    std::function<void()>&& new_thread_callback,
    size_t starting_threads,
    size_t max_threads
)
    : m_core(
        CONSTRUCT_TOKEN,
        std::move(new_thread_callback),
        starting_threads,
        max_threads
    )
{}
ParallelTaskRunner::~ParallelTaskRunner() = default;
size_t ParallelTaskRunner::current_threads() const{
    return m_core->current_threads();
}
size_t ParallelTaskRunner::max_threads() const{
    return m_core->max_threads();
}
WallDuration ParallelTaskRunner::cpu_time() const{
    return m_core->cpu_time();
}
void ParallelTaskRunner::ensure_threads(size_t threads){
    m_core->ensure_threads(threads);
}
//void ParallelTaskRunner::wait_for_everything(){
//    m_core->wait_for_everything();
//}
std::unique_ptr<AsyncTask> ParallelTaskRunner::blocking_dispatch(std::function<void()>&& func){
    return m_core->blocking_dispatch(std::move(func));
}
std::unique_ptr<AsyncTask> ParallelTaskRunner::try_dispatch(std::function<void()>& func){
    return m_core->try_dispatch(func);
}
void ParallelTaskRunner::run_in_parallel(
    const std::function<void(size_t index)>& func,
    size_t start, size_t end,
    size_t block_size
){
    m_core->run_in_parallel(func, start, end, block_size);
}








ParallelTaskRunnerCore::ParallelTaskRunnerCore(
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
ParallelTaskRunnerCore::~ParallelTaskRunnerCore(){
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
        task->signal();
    }
}

WallDuration ParallelTaskRunnerCore::cpu_time() const{
    //  TODO: Don't lock the entire queue.
    WallDuration ret = WallDuration::zero();
    std::lock_guard<std::mutex> lg(m_lock);
    for (const ThreadData& thread : m_threads){
//        ret += thread_cpu_time(thread.handle);
        ret += thread.runtime.total();
    }
    return ret;
}


void ParallelTaskRunnerCore::ensure_threads(size_t threads){
    std::lock_guard<std::mutex> lg(m_lock);
    while (m_threads.size() < threads){
        spawn_thread();
    }
}
#if 0
void ParallelTaskRunnerCore::wait_for_everything(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count == 0;
    });
}
#endif

std::unique_ptr<AsyncTask> ParallelTaskRunnerCore::blocking_dispatch(std::function<void()>&& func){
    std::unique_ptr<AsyncTask> task(new AsyncTask(std::move(func)));

    std::unique_lock<std::mutex> lg(m_lock);

    m_dispatch_cv.wait(lg, [this]{
        return m_queue.size() + m_busy_count < m_max_threads;
    });

    //  Enqueue task.
    m_queue.emplace_back(task.get());

    if (m_queue.size() + m_busy_count > m_threads.size()){
        spawn_thread();
    }

    m_thread_cv.notify_one();

    return task;
}
std::unique_ptr<AsyncTask> ParallelTaskRunnerCore::try_dispatch(std::function<void()>& func){
    std::lock_guard<std::mutex> lg(m_lock);

    if (m_queue.size() + m_busy_count >= m_max_threads){
        return nullptr;
    }

    std::unique_ptr<AsyncTask> task(new AsyncTask(std::move(func)));

    //  Enqueue task.
    m_queue.emplace_back(task.get());

    if (m_queue.size() + m_busy_count > m_threads.size()){
        spawn_thread();
    }

    m_thread_cv.notify_one();

    return task;
}


void ParallelTaskRunnerCore::run_in_parallel(
    const std::function<void(size_t index)>& func,
    size_t start, size_t end,
    size_t block_size
){
    if (start >= end){
        return;
    }
    size_t total = end - start;

    if (block_size == 0){
        block_size = total / m_max_threads / 2;
        if (block_size == 0){
            block_size = 1;
        }
    }

    size_t blocks = (total + block_size - 1) / block_size;

    std::vector<std::unique_ptr<AsyncTask>> tasks;
    for (size_t c = 0; c < blocks; c++){
        tasks.emplace_back(blocking_dispatch([=, &func]{
            size_t s = start + c * block_size;
            size_t e = std::min(s + block_size, end);
//            cout << "Running: [" << s << "," << e << ")" << endl;
            for (; s < e; s++){
                func(s);
            }
        }));
    }

    for (std::unique_ptr<AsyncTask>& task : tasks){
        task->wait_and_rethrow_exceptions();
    }
}



void ParallelTaskRunnerCore::spawn_thread(){
    ThreadData& handle = m_threads.emplace_back();
    handle.thread = std::thread(
        run_with_catch,
        "ParallelTaskRunner::thread_loop()",
        [&, this]{ thread_loop(handle); }
    );
}
void ParallelTaskRunnerCore::thread_loop(ThreadData& data){
    data.handle = current_thread_handle();

    if (m_new_thread_callback){
        m_new_thread_callback();
    }

    data.runtime.start();

    bool busy = false;
    while (true){
        AsyncTask* task;
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
                data.runtime.stop();
                m_thread_cv.wait(lg);
                data.runtime.start();
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
        }
        task->signal();
    }
}




//template class Pimpl<ParallelTaskRunnerCore>;






}
