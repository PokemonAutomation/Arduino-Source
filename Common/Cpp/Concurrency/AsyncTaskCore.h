/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AsyncTaskCore_H
#define PokemonAutomation_AsyncTaskCore_H

#include <functional>
#include <atomic>
#include <exception>
#include "SpinPause.h"
#include "Mutex.h"
#include "ConditionVariable.h"

namespace PokemonAutomation{



class AsyncTaskCore{
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
    ~AsyncTaskCore(){
        State state = m_state.load(std::memory_order_acquire);
        if (state == State::NOT_STARTED || state == State::SAFE_TO_DESTRUCT){
    //        cout << "Already Done: " << (int)state << endl;
            return;
        }

        {
            std::unique_lock<Mutex> lg(m_lock);
            m_cv.wait(lg, [this]{
                return m_state.load(std::memory_order_relaxed) != State::RUNNING;
            });
        }

        while (m_state.load(std::memory_order_acquire) != State::SAFE_TO_DESTRUCT){
            pause();
        }
    }
    AsyncTaskCore(std::function<void()> task)
        : m_task(std::move(task))
        , m_state(State::NOT_STARTED)
    {}

    bool is_finished() const noexcept{
        State state = m_state.load(std::memory_order_acquire);
        return state == State::FINISHED || state == State::SAFE_TO_DESTRUCT;
    }

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions(){
        if (!is_finished()){
            std::unique_lock<Mutex> lg(m_lock);
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
        m_state.store(State::RUNNING, std::memory_order_release);
    }
    void report_cancelled() noexcept{
        {
            m_state.store(State::FINISHED, std::memory_order_release);
            {
                std::lock_guard<Mutex> lg(m_lock);
            }
            m_cv.notify_all();
        }
        m_state.store(State::SAFE_TO_DESTRUCT, std::memory_order_release);
    }
    void run() noexcept{
        {
            try{
                m_task();
            }catch (...){
                std::lock_guard<Mutex> lg(m_lock);
                m_exception = std::current_exception();
            }
        }
        report_cancelled();
    }


private:
    std::function<void()> m_task;
    std::atomic<State> m_state;

    std::exception_ptr m_exception;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
};



}
#endif
