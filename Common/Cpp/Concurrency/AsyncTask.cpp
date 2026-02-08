/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include <exception>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "SpinPause.h"
#include "Mutex.h"
#include "ConditionVariable.h"
#include "AsyncTask.h"
#include "AsyncTaskCore.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




AsyncTask::AsyncTask(AsyncTask&&) = default;
AsyncTask& AsyncTask::operator=(AsyncTask&&) = default;


AsyncTask::AsyncTask()
    : m_sanitizer("AsyncTask")
{}
AsyncTask::AsyncTask(std::function<void()> task)
    : m_core(CONSTRUCT_TOKEN, std::move(task))
    , m_sanitizer("AsyncTask")
{}

AsyncTask::~AsyncTask(){
    wait_and_ignore_exceptions();
}


bool AsyncTask::is_finished() const noexcept{
#ifdef PA_SANITIZE_AsyncTask
    auto scope = m_sanitizer.check_scope();
#endif
    return m_core->is_finished();
}

void AsyncTask::wait_and_ignore_exceptions() noexcept{
    m_core.clear();
}
void AsyncTask::wait_and_rethrow_exceptions(){
#ifdef PA_SANITIZE_AsyncTask
    auto scope = m_sanitizer.check_scope();
#endif
    m_core->wait_and_rethrow_exceptions();
}


void AsyncTask::report_started(){
#ifdef PA_SANITIZE_AsyncTask
    auto scope = m_sanitizer.check_scope();
#endif
    m_core->report_started();
}
void AsyncTask::report_cancelled() noexcept{
    m_sanitizer.check_usage();
    m_core->report_cancelled();
}
void AsyncTask::run() noexcept{
    m_sanitizer.check_usage();
    m_core->run();
}





}
