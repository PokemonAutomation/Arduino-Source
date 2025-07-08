/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SpinPause.h"
#include "AsyncTask.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



AsyncTask::~AsyncTask(){
    State state = m_state.load(std::memory_order_acquire);
    if (state == State::NOT_STARTED || state == State::SAFE_TO_DESTRUCT){
//        cout << "Already Done: " << (int)state << endl;
        return;
    }

    {
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.wait(lg, [this]{
            return m_state.load(std::memory_order_relaxed) != State::RUNNING;
        });
    }

    while (m_state.load(std::memory_order_acquire) != State::SAFE_TO_DESTRUCT){
        pause();
    }

//    cout << "Late Finish" << endl;
}


void AsyncTask::report_cancelled() noexcept{
    {
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        m_state.store(State::FINISHED, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lg(m_lock);
        }
        m_cv.notify_all();
    }
    m_state.store(State::SAFE_TO_DESTRUCT, std::memory_order_release);
}
void AsyncTask::run() noexcept{
    {
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        try{
            m_task();
        }catch (...){
            std::lock_guard<std::mutex> lg(m_lock);
            m_exception = std::current_exception();
        }
    }
    report_cancelled();
}





}
