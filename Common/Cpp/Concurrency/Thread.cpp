/*  Thread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Thread.h"

namespace PokemonAutomation{




struct Thread::Data{
//    std::mutex m_lock;
//    std::condition_variable m_cv;
    std::atomic<bool> m_stopped;
    std::thread m_thread;

    Data(std::function<void()>&& function)
        : m_stopped(false)
        , m_thread(
            [this, function = std::move(function)]{
                function();
//                std::lock_guard<std::mutex> lg(m_lock);
//                m_stopped = true;
//                m_cv.notify_all();
                m_stopped.store(true, std::memory_order_release);
            }
        )
    {}
};




Thread::Thread(Thread&&) = default;
Thread& Thread::operator=(Thread&&) = default;

Thread::~Thread(){
    join();
}
Thread::Thread(std::function<void()>&& function)
    : m_data(CONSTRUCT_TOKEN, std::move(function))
{}
void Thread::join(){
    if (!m_data){
        return;
    }

    Data& data = m_data;

#if 0
    //
    //  Even std::condition_variable is broken!
    //
//    std::unique_lock<std::mutex> lg(data.m_lock);
//    data.m_cv.wait(lg, [&data]{
//        return data.m_stopped;
//    });

    while (!data.m_stopped.load(std::memory_order_acquire)){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    //  Here we detach() instead of join() due to Qt bug.
    //  It is safe because the thread no longer touches anything in this class.
    data.m_thread.detach();
#else
    //  Fuck it, just stick with Qt 6.8.3 for now.
    data.m_thread.join();
#endif

    m_data.clear();
}




}
