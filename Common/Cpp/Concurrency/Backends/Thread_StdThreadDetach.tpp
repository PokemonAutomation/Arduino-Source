/*  Thread (std::thread detach)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class is a wrapper for std::thread that tries to work around Qt thread issue at 6.9+:
 *      https://github.com/PokemonAutomation/Arduino-Source/issues/570
 *      https://bugreports.qt.io/browse/QTBUG-131892
 *
 */

#include <thread>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/Thread.h"

namespace PokemonAutomation{




// PIMPL implementation - hides std::thread from header
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




// Move semantics - same as std::thread (move-only, no copy)
Thread::Thread(Thread&&) = default;
Thread& Thread::operator=(Thread&&) = default;

// CRITICAL: Destructor automatically joins thread (unlike std::thread)
// - std::thread calls std::terminate() if destroyed while joinable (crashes!)
// - This Thread safely waits for thread to complete before destroying
// - Makes Thread safer and more convenient to use in RAII patterns
Thread::~Thread(){
    join();
}

// Default constructor - creates empty Thread with no underlying thread
Thread::Thread() = default;

// Main constructor - starts thread immediately with given function
// - Uses CONSTRUCT_TOKEN to construct Pimpl<Data> with Data constructor
// - Thread starts running immediately (same as std::thread behavior)
Thread::Thread(std::function<void()>&& function)
    : m_data(CONSTRUCT_TOKEN, std::move(function))
{}

void Thread::join(){
    // Early return if Thread is default-constructed, moved-from or already joined
    if (!m_data){
        return;
    }

    Data& data = m_data;


    // ========================================================================
    // ATTEMPTED WORKAROUND CODE FOR Qt 6.9+ BUG
    // ========================================================================

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

    // Clear the Pimpl data, marking this Thread as joined/empty
    // After this, operator bool() returns false and join() becomes no-op
    m_data.clear();
}


// Check if thread is joinable (same semantics as std::thread::joinable)
// - Returns false if Thread is default-constructed, moved-from
// - Returns false after join() completes (m_data is cleared)
// - Otherwise delegates to underlying std::thread::joinable()
bool Thread::joinable() const{
    return m_data && m_data->m_thread.joinable();
}



}
