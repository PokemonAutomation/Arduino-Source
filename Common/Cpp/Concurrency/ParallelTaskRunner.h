/*  Parallel Task Runner
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ParallelTaskRunner_H
#define PokemonAutomation_ParallelTaskRunner_H

#include "AsyncDispatcher.h"

namespace PokemonAutomation{


class ParallelTaskRunner{
public:
    ParallelTaskRunner(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads
    );
    ~ParallelTaskRunner();

    void wait_for_everything();

    std::shared_ptr<AsyncTask> dispatch(std::function<void()>&& func);

    void run_in_parallel(
        const std::function<void(size_t index)>& func,
        size_t start, size_t end,
        size_t block_size = 0
    );


private:
//    void dispatch_task(AsyncTask& task);
    void thread_loop();


private:
    std::function<void()> m_new_thread_callback;
    size_t m_max_threads;
    std::deque<std::shared_ptr<AsyncTask>> m_queue;
    std::vector<std::thread> m_threads;
    bool m_stopping;
    size_t m_busy_count;
    std::mutex m_lock;
    std::condition_variable m_thread_cv;
    std::condition_variable m_dispatch_cv;
};





}
#endif
