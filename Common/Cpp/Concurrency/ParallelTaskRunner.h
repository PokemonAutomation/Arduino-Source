/*  Parallel Task Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ParallelTaskRunner_H
#define PokemonAutomation_ParallelTaskRunner_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "AsyncDispatcher.h"

namespace PokemonAutomation{


class ParallelTaskRunnerCore;


class ParallelTaskRunner final{
public:
    ParallelTaskRunner(
        std::function<void()>&& new_thread_callback,
        size_t starting_threads,
        size_t max_threads
    );
    ~ParallelTaskRunner();

    size_t current_threads() const;
    size_t max_threads() const;
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
    Pimpl<ParallelTaskRunnerCore> m_core;
};









}
#endif
