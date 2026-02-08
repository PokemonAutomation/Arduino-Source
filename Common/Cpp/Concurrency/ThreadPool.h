/*  Thread Pool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This is a thread pool for compute-heavy tasks.
 *
 *  This thread pool has a limited number of threads and should only be used for
 *  compute-heavy tasks that do not block or yield. A blocked thread will still
 *  count towards the thread limit.
 *
 *  Because the # of threads is capped, it is safe to spam this thread pool with
 *  lots of smaller tasks.
 *
 */

#ifndef PokemonAutomation_ThreadPool_H
#define PokemonAutomation_ThreadPool_H

#include <functional>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{

class AsyncTask;


class ThreadPool{
public:
    virtual void stop() = 0;
    virtual void ensure_threads(size_t threads) = 0;

public:
    virtual size_t current_threads() const = 0;
    virtual size_t max_threads() const = 0;
    virtual WallDuration cpu_time() const = 0;

public:
    //  As of this writing, tasks dispatched earlier are not allowed to block
    //  on tasks that are dispatched later as it may cause a deadlock.

    //  Dispatch the function and return immediately.
    //  The function is not guaranteed to begin running immediately.
    [[nodiscard]] virtual AsyncTask dispatch(std::function<void()>&& func) = 0;

    //  Dispatch the function and begin running now.
    //  If there are no threads available, it waits until there are.
    //  This function will not return until the function has begin running.
    [[nodiscard]] virtual AsyncTask dispatch_now_blocking(std::function<void()>&& func) = 0;

    //  Try to dispatch the function and begin running now.
    //  Returns null if no threads are available. "func" will be moved-from only on success.
    //  This function returns immediately.
    [[nodiscard]] virtual AsyncTask try_dispatch_now(std::function<void()>& func) = 0;

    //  Run function for all the indices [start, end).
    //  Lower indices are not allowed to block on higher indices.
    virtual void run_in_parallel(
        const std::function<void(size_t index)>& func,
        size_t start, size_t end,
        size_t block_size = 0
    ) = 0;
};





}
#endif
