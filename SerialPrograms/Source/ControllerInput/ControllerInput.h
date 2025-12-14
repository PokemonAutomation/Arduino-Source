/*  Controller Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_ControllerInput_H
#define PokemonAutomation_ControllerInput_ControllerInput_H

#include "Common/Compiler.h"
#include "Common/Cpp/ListenerSet.h"

namespace PokemonAutomation{


enum class ControllerInputType{
    HID_Keyboard,
    StandardGamepad,
};



class ControllerInputState{
public:
    ControllerInputState(ControllerInputType type)
        : m_type(type)
    {}

    ControllerInputType type() const{
        return m_type;
    }

    virtual void clear() = 0;
    virtual bool is_neutral() const = 0;
    virtual bool operator==(const ControllerInputState& state) const = 0;
    bool operator!=(const ControllerInputState& state) const{
        return !operator==(state);
    }

private:
    const ControllerInputType m_type;
};



struct ControllerInputListener{
    virtual void run_controller_input(ControllerInputState& state){}
};





class ControllerInputSource{
public:
    //  This called before program shutdown. Child classes should shutdown any
    //  threads they own to avoid static-deinit shenanigans.
    virtual void stop() = 0;

    //  User has switched away from the current console.
    //  Release all buttons and reset back to neutral state.
    virtual void clear_state() = 0;

    //  These are called when controller inputs are enabled/disabled.
    void add_listener(ControllerInputListener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(ControllerInputListener& listener){
        m_listeners.remove(listener);
    }

protected:
    ListenerSet<ControllerInputListener> m_listeners;
};



//  This is called at the end of main() before program shutdown. This allows all
//  inputs to end their threads to avoid static-deinit shenanigans.
void global_input_stop();

//  This is called whenever the user switches away from the current console.
//  Release all buttons and reset back to neutral state.
void global_input_clear_state();

//  These are called when controller inputs are enabled/disabled as a result
//  of switching to/from a console or starting/stopping a program.
void global_input_add_listener(ControllerInputListener& listener);
void global_input_remove_listener(ControllerInputListener& listener);











}
#endif









