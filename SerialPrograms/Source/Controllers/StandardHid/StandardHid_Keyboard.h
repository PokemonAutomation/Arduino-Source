/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the raw (full) controller API that is exposed to programs.
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_H
#define PokemonAutomation_StandardHid_Keyboard_H

#include <vector>
#include "Common/Cpp/Containers/Pimpl.h"
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"
#include "Controllers/Controller.h"

namespace PokemonAutomation{
namespace StandardHid{


class Keyboard;
using KeyboardContext = ControllerContext<Keyboard>;




class Keyboard : public AbstractController{
public:
    using ContextType = KeyboardContext;

    Keyboard(Logger& logger);
    virtual ~Keyboard();


public:
    static const char NAME[];
    virtual const char* name() override{
        return NAME;
    };


public:
    //  Standard Commands

    //  Note that only 6 keys (not counting modifiers) can be communicated at
    //  once over the HID protocol. If you hold down more than 6, only the first
    //  6 chronologically will be applied.

    virtual void issue_key(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        KeyboardKey key
    ) = 0;
    virtual void issue_keys(
        Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const std::vector<KeyboardKey>& keys
    ) = 0;


public:
    //  Controller Input

    virtual void run_controller_input(const ControllerInputState& state) override;


private:
};




}
}
#endif
