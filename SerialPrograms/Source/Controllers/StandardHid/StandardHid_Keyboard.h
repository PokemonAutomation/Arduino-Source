/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the raw (full) controller API that is exposed to programs.
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_H
#define PokemonAutomation_StandardHid_Keyboard_H

#include <set>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Controllers/Controller.h"
#include "StandardHid_Keyboard_ControllerButtons.h"

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


protected:
    //  Must call before destruction begins.
    void stop() noexcept;


public:
    //  Standard Commands

    virtual void issue_key(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        KeyboardKey key
    ) = 0;
    virtual void issue_keys(
        const Cancellable* cancellable,
        Milliseconds delay, Milliseconds hold, Milliseconds cooldown,
        const std::set<KeyboardKey>& keys
    ) = 0;


public:
    //  Keyboard Input

    virtual void keyboard_release_all() override;
    virtual void keyboard_press(const QKeyEvent& event) override;
    virtual void keyboard_release(const QKeyEvent& event) override;


private:
    class KeyboardManager;
    Pimpl<KeyboardManager> m_keyboard_manager;
};




}
}
#endif
