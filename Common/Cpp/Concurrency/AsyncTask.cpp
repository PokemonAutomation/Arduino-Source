/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "AsyncTask.h"

namespace PokemonAutomation{



AsyncTask::~AsyncTask(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_finished; });
}
bool AsyncTask::is_finished() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_finished;
}
void AsyncTask::rethrow_exceptions(){
    if (!m_stopped_with_error.load(std::memory_order_acquire)){
        return;
    }
    std::unique_lock<std::mutex> lg(m_lock);
    if (m_exception){
        std::rethrow_exception(m_exception);
    }
}
void AsyncTask::wait_and_rethrow_exceptions(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_finished; });
    if (m_exception){
        std::rethrow_exception(m_exception);
    }
}
void AsyncTask::signal(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_finished = true;
    m_cv.notify_all();
}





}
