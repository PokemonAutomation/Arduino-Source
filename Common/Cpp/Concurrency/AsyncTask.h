/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AsyncTask_H
#define PokemonAutomation_AsyncTask_H

#include <memory>

#define PA_SANITIZE_AsyncTask

#ifdef PA_SANITIZE_AsyncTask
#include "Common/Cpp/LifetimeSanitizer.h"
#endif

namespace PokemonAutomation{


class AsyncTaskCore{
public:
    virtual ~AsyncTaskCore() = default;
    virtual bool is_finished() const noexcept = 0;
    virtual void wait_and_rethrow_exceptions() = 0;

public:
    //  These should only be called inside a parallel framework.
    //  These are not thread-safe with each other.
    virtual void report_started() = 0;
    virtual void report_cancelled() noexcept = 0;
    virtual void run() noexcept = 0;
};



class AsyncTask{
public:
    AsyncTask(const AsyncTask&) = delete;
    void operator=(const AsyncTask&) = delete;
    AsyncTask(AsyncTask&&) = default;
    AsyncTask& operator=(AsyncTask&&) = default;

    //  If the task has already started, this will wait for it to finish.
    //  This will not rethrow exceptions.
    ~AsyncTask() = default;


public:
    AsyncTask()
        : m_sanitizer("AsyncTask")
    {}
    AsyncTask(std::unique_ptr<AsyncTaskCore> task)
        : m_task(std::move(task))
        , m_sanitizer("AsyncTask")
    {}

    operator bool() const{
        return (bool)m_task;
    }
    AsyncTaskCore* core(){
        return m_task.get();
    }
    bool is_finished() const noexcept{
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        return m_task->is_finished();
    }

    //  Wait for the task to finish. Will ignore any exceptions.
    void wait_and_ignore_exceptions() noexcept{
        m_task.reset();
    }

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions(){
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        m_task->wait_and_rethrow_exceptions();
    }


public:
    //  These should only be called inside a parallel framework.
    //  These are not thread-safe with each other.
    void report_started(){
#ifdef PA_SANITIZE_AsyncTask
        auto scope = m_sanitizer.check_scope();
#endif
        m_task->report_started();
    }
    void report_cancelled() noexcept{
        m_sanitizer.check_usage();
        m_task->report_cancelled();
    }
    void run() noexcept{
        m_sanitizer.check_usage();
        m_task->run();
    }


private:
    std::unique_ptr<AsyncTaskCore> m_task;

#ifdef PA_SANITIZE_AsyncTask
    LifetimeSanitizer m_sanitizer;
#endif
};



}
#endif
