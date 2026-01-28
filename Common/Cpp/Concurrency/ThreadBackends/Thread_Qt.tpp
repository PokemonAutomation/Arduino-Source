/*  Thread (C++)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QThread>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/Thread.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




// PIMPL implementation - hides std::thread from header
struct Thread::Data : public QThread{
    Data(std::function<void()>&& function)
        : m_function(std::move(function))
        , m_finished(false)
    {
        start();
    }
    ~Data(){
//        cout << "quitting: " << this << endl;
//        quit();
//        cout << "waiting: " << this << endl;
//        while (!m_finished.load(std::memory_order_acquire)){
//            wait(1000);
//        }

        //  What the fuck?!?! This hangs on Qt6.9 even after run() returns!
        wait();

//        cout << "finished: " << this << endl;
    }

    virtual void run() override{
//        cout << "starting: " << this << endl;
        m_function();
        m_finished.store(true, std::memory_order_acquire);
//        cout << "exiting: " << this << endl;
    }

    std::function<void()> m_function;

    std::atomic<bool> m_finished;
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

    // Clear the Pimpl data, marking this Thread as joined/empty
    // After this, operator bool() returns false and join() becomes no-op
    m_data.clear();
}


// Check if thread is joinable (same semantics as std::thread::joinable)
// - Returns false if Thread is default-constructed, moved-from
// - Returns false after join() completes (m_data is cleared)
// - Otherwise delegates to underlying std::thread::joinable()
bool Thread::joinable() const{
    return m_data;
}



}
