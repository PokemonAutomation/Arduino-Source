/*  Keyboard Event Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_Controllers_KeyboardEventHandler_H
#define PokemonAutomation_Controllers_KeyboardEventHandler_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/LifetimeSanitizer.h"

namespace PokemonAutomation{

class ControllerState;  // forward declaration to avoid circular dependency

class KeyboardEventHandler{
public:
    KeyboardEventHandler();
    virtual ~KeyboardEventHandler();

    struct KeyboardListener{
        virtual void on_keyboard_command_sent(WallClock time_stamp, const ControllerState& state) = 0;
        virtual void on_keyboard_command_stopped(WallClock time_stamp) = 0;
    };
    void add_listener(KeyboardListener& listener);
    void remove_listener(KeyboardListener& listener);

protected:
    //  Report that the keyboard state has changed. This will be pushed to
    //  all listeners.
    void report_keyboard_command_sent(WallClock time_stamp, const ControllerState& state);

    void report_keyboard_command_stopped(WallClock time_stamp);

private:
    struct Data;
    Pimpl<Data> m_data;

    LifetimeSanitizer m_lifetime_sanitizer;
};

}
#endif