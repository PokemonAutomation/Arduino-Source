/*  Fire and Forget Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "FireForgetDispatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


FireForgetDispatcher global_dispatcher;



FireForgetDispatcher::FireForgetDispatcher()
    : m_stopping(false)
{}
FireForgetDispatcher::~FireForgetDispatcher(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    if (m_thread.joinable()){
        m_thread.join();
    }
}
void FireForgetDispatcher::dispatch(std::function<void()>&& func){
    std::lock_guard<std::mutex> lg(m_lock);
    m_queue.emplace_back(std::move(func));
    m_cv.notify_one();

    //  Lazy create thread.
    if (!m_thread.joinable()){
        m_thread = std::thread(run_with_catch, "FireForgetDispatcher::thread_loop()", [this]{ thread_loop(); });
    }
}

void FireForgetDispatcher::thread_loop(){
    while (true){
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lg(m_lock);
            if (m_stopping){
                return;
            }
            if (m_queue.empty()){
                m_cv.wait(lg);
                continue;
            }

            task = std::move(m_queue.front());
            m_queue.pop_front();
        }

        task();
    }
}



}
