/*  Thread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  This class is a wrapper for std::thread that works around:
 *      https://github.com/PokemonAutomation/Arduino-Source/issues/570
 *      https://bugreports.qt.io/browse/QTBUG-131892
 *
 *  And since we're wrapping it, we might as well throw everything behind a
 *  PIMPL to improve compilation times.
 *
 */

#ifndef PokemonAutomation_Thread_H
#define PokemonAutomation_Thread_H

#include <functional>
#include "Common/Cpp/Containers/Pimpl.h"

namespace PokemonAutomation{


class Thread{
public:
    ~Thread();
    Thread(Thread&&);
    Thread& operator=(Thread&&);

public:
    Thread() = default;
    Thread(std::function<void()>&& function);

    operator bool() const{
        return m_data;
    }
    void join();

private:
    enum class State;

    struct Data;
    Pimpl<Data> m_data;
};





}
#endif
