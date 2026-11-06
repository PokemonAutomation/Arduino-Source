/*  Standard Gamepad Input State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_Gamepad_GamepadInput_State_H
#define PokemonAutomation_ControllerInput_Gamepad_GamepadInput_State_H

#include "ControllerInput/ControllerInput.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProControllerState.h"

namespace PokemonAutomation{

class GamepadInputState final : public ControllerInputState{
public:
    GamepadInputState()
        : ControllerInputState(ControllerInputType::StandardGamepad)
    {}

    GamepadInputState(const GamepadInputState&) = default;

    GamepadInputState& operator=(const GamepadInputState& other){
        controller_state = other.controller_state;
        return *this;
    }

    virtual void clear() override{
        controller_state.clear();
    }
    virtual bool is_neutral() const override{
        return controller_state.is_neutral();
    }
    virtual bool operator==(const ControllerInputState& state) const override{
        const auto* other = dynamic_cast<const GamepadInputState*>(&state);
        if (other == nullptr){
            return false;
        }
        return controller_state == other->controller_state;
    }

    NintendoSwitch::ProControllerState controller_state;
};

class GamepadInputGate{
public:
    void reset(){
        m_armed = false;
    }
    void filter(GamepadInputState& state, bool has_listeners){
        if (!has_listeners){
            m_armed = false;
        }
        if (!m_armed){
            m_armed = has_listeners && state.is_neutral();
            state.clear();
        }
    }

private:
    bool m_armed = false;
};

}
#endif
