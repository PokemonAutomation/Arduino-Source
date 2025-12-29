/*  Thread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  This class is a wrapper for std::thread that tries to work around Qt thread issue at 6.9+:
 *      https://github.com/PokemonAutomation/Arduino-Source/issues/570
 *      https://bugreports.qt.io/browse/QTBUG-131892
 *
 *  Same usage as std::thread except automatic join on destruction:
 *     - std::thread: Calls std::terminate() if destroyed while joinable (unsafe!)
 *     - Thread: Automatically calls join() in destructor (safe, prevents crashes)
 *  Thread also allows multiple join() calls from the main thread where Thread is created.
 *  Subsequent join() calls will just be no-op.
 *
 *  It still has the same caveat:
 *     - Unsafe when calling bool(), join() or joinable() from multiple threads at same time.
 *       Prefer to call them from the main thread where Thread is created.
 */

#ifndef PokemonAutomation_Thread_H
#define PokemonAutomation_Thread_H

#include <functional>
#include "Common/Cpp/Containers/Pimpl.h"

namespace PokemonAutomation{


class Thread{
public:
    // Destructor - automatically joins thread (RAII safety)
    ~Thread();

    // Move-only semantics (same as std::thread)
    Thread(Thread&&);
    Thread& operator=(Thread&&);

public:
    // Default constructor - creates empty Thread with no underlying thread
    Thread();

    // Constructor - starts thread immediately with given function like std::thread
    // Function is moved (not copied) for efficiency
    Thread(std::function<void()>&& function);

    // Returns true if Thread has valid data (not default-constructed or moved-from)
    // and not joined.
    operator bool() const{
        return m_data;
    }

    // Join thread - waits for completion (can call multiple times safely).
    void join();

    // Check if thread has valid data (not default-constructed or moved-from) and
    // is joinable (active and not yet joined).
    bool joinable() const;

private:
    enum class State;  // Reserved for future use

    // PIMPL data - defined in .cpp to hide implementation details
    struct Data;
    Pimpl<Data> m_data;
};





}
#endif
