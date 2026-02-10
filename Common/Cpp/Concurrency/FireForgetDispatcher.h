/*  Fire and Forget Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FireForgetDispatcher_H
#define PokemonAutomation_FireForgetDispatcher_H

#include <deque>
#include <functional>
#include "Mutex.h"
#include "ConditionVariable.h"
#include "Thread.h"

namespace PokemonAutomation{


class FireForgetDispatcher{
public:
    FireForgetDispatcher();
    ~FireForgetDispatcher();

    //  Dispatch the specified task and return a handle to it.
    //  Call "handle->wait()" to wait for the task to finish.
    void dispatch(std::function<void()>&& func);

    void stop();


private:
    void thread_loop();

private:
    std::deque<std::function<void()>> m_queue;
    Thread m_thread;
    bool m_stopping;
    Mutex m_lock;
    ConditionVariable m_cv;
};


extern FireForgetDispatcher global_dispatcher;





}
#endif

