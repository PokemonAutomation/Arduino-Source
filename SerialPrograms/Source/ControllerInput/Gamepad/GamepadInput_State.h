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
    GamepadInputState();

    GamepadInputState(const GamepadInputState&) = default;

    GamepadInputState& operator=(const GamepadInputState& other);

    virtual void clear() override;
    virtual bool is_neutral() const override;
    virtual bool operator==(const ControllerInputState& state) const override;

    NintendoSwitch::ProControllerState controller_state;
};

class GamepadInputGate{
public:
    void reset();
    void filter(GamepadInputState& state, bool has_listeners);

private:
    bool m_armed = false;
};

}
#endif
