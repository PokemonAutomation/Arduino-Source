/*  Fire and Forget Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FireForgetDispatcher_H
#define PokemonAutomation_FireForgetDispatcher_H

#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace PokemonAutomation{


class FireForgetDispatcher{
public:
    FireForgetDispatcher();
    ~FireForgetDispatcher();

    //  Dispatch the specified task and return a handle to it.
    //  Call "handle->wait()" to wait for the task to finish.
    void dispatch(std::function<void()>&& func);


private:
    void thread_loop();

private:
    std::deque<std::function<void()>> m_queue;
    std::thread m_thread;
    bool m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
};


extern FireForgetDispatcher global_dispatcher;





}
#endif

