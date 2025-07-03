/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AsyncTask_H
#define PokemonAutomation_AsyncTask_H

#include <functional>
#include <atomic>
#include <mutex>

namespace PokemonAutomation{


class AsyncTask{
public:
    //  Wait for the task to finish before destructing. Doesn't rethrow exceptions.
    ~AsyncTask();

    bool is_finished() const;

    //  If the task ended with an exception, rethrow it here.
    //  This does not clear the exception.
    void rethrow_exceptions();

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions();


private:
    template <class... Args>
    AsyncTask(Args&&... args)
        : m_task(std::forward<Args>(args)...)
        , m_finished(false)
        , m_stopped_with_error(false)
    {}
    void signal();

private:
    friend class FireForgetDispatcher;
    friend class AsyncDispatcher;
    friend class ComputationThreadPoolCore;

    std::function<void()> m_task;
    bool m_finished;
    std::atomic<bool> m_stopped_with_error;
    std::exception_ptr m_exception;
    mutable std::mutex m_lock;
    std::condition_variable m_cv;
};



}
#endif
