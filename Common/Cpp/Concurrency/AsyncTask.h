/*  Async Task
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AsyncTask_H
#define PokemonAutomation_AsyncTask_H

#include <functional>
#include "Common/Cpp/Containers/Pimpl.h"

#define PA_SANITIZE_AsyncTask

#ifdef PA_SANITIZE_AsyncTask
#include "Common/Cpp/LifetimeSanitizer.h"
#endif

namespace PokemonAutomation{

class AsyncTaskCore;


class AsyncTask{
public:
    AsyncTask(const AsyncTask&) = delete;
    void operator=(const AsyncTask&) = delete;
    AsyncTask(AsyncTask&&);
    AsyncTask& operator=(AsyncTask&&);

    //  If the task has already started, this will wait for it to finish.
    //  This will not rethrow exceptions.
    ~AsyncTask();


public:
    AsyncTask();
    AsyncTask(std::function<void()> task);

    operator bool() const{
        return m_core;
    }
    AsyncTaskCore* core(){
        return m_core.get();
    }
    bool is_finished() const noexcept;

    //  Wait for the task to finish. Will ignore any exceptions.
    void wait_and_ignore_exceptions() noexcept;

    //  Wait for the task to finish. Will rethrow any exceptions.
    void wait_and_rethrow_exceptions();


public:
    //  These should only be called inside a parallel framework.
    //  These are not thread-safe with each other.
    void report_started();
    void report_cancelled() noexcept;
    void run() noexcept;


private:
    Pimpl<AsyncTaskCore> m_core;

#ifdef PA_SANITIZE_AsyncTask
    LifetimeSanitizer m_sanitizer;
#endif
};



}
#endif
