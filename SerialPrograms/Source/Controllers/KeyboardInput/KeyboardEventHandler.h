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

namespace NintendoSwitch{
    class ProControllerState;  // forward declaration to avoid circular dependency
}

class KeyboardEventHandler{
public:
    KeyboardEventHandler();
    virtual ~KeyboardEventHandler();

     struct KeyboardListener{
        virtual void on_keyboard_control_state_change(Milliseconds duration, const NintendoSwitch::ProControllerState& state){}
    };
    void add_listener(KeyboardListener& listener);
    void remove_listener(KeyboardListener& listener);

private:
    struct Data;
    Pimpl<Data> m_data;

    LifetimeSanitizer m_lifetime_sanitizer;
};

}
#endif