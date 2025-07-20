/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AsyncTask_H
#define PokemonAutomation_AsyncTask_H

#include <functional>
#include <atomic>
#include <exception>
#include <mutex>
#include <condition_variable>

#define PA_SANITIZE_AsyncTask

#ifdef PA_SANITIZE_AsyncTask
#include "Common/Cpp/LifetimeSanitizer.h"
#endif

namespace PokemonAutomation{


class AsyncTask{
public:
    enum class State{
        NOT_STARTED,
        RUNNING,
        FINISHED,
        SAFE_TO_DESTRUCT,
    };


public:
    //  If the task has already started, this will wait for it to finish.
    //  This will not rethrow exceptions.
    ~AsyncTask();


public:
    template <class... Args>
    AsyncTask(Args&&... args)
        : m_task(std::forward<Args>(args)...)
        , m_state(State::NOT_STARTED)
    {}

    bool is_finished() const noexcept{
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        State state = m_state.load(std::memory_order_acquire);
        return state == State::FINISHED || state == State::SAFE_TO_DESTRUCT;
    }

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions(){
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        if (!is_finished()){
            std::unique_lock<std::mutex> lg(m_lock);
            m_cv.wait(lg, [this]{ return is_finished(); });
        }
        if (m_exception){
            std::rethrow_exception(m_exception);
        }
    }


public:
    //  These should only be called inside a parallel framework.
    //  These are not thread-safe with each other.
    void report_started(){
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        m_state.store(State::RUNNING, std::memory_order_release);
    }
    void report_cancelled() noexcept;
    void run() noexcept;


private:
    std::function<void()> m_task;
    std::atomic<State> m_state;
    std::exception_ptr m_exception;
    mutable std::mutex m_lock;
    std::condition_variable m_cv;

#ifdef PA_SANITIZE_AsyncTask
    LifetimeSanitizer m_sanitizer;
#endif
};



}
#endif
