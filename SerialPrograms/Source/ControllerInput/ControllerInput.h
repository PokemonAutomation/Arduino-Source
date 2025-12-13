/*  Controller Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_ControllerInput_H
#define PokemonAutomation_ControllerInput_ControllerInput_H

#include "Common/Compiler.h"

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
    virtual void controller_input_state(ControllerInputState& state){}
};




}
#endif
