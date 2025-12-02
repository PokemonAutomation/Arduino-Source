/*  Nintendo Switch Joycon State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_JoyconState_H
#define PokemonAutomation_NintendoSwitch_JoyconState_H

#include "Controllers/ControllerState.h"
#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class JoyconController;



class JoyconState : public ControllerState{
public:
    using ControllerType = JoyconController;

public:
    virtual void clear() override;
    virtual bool operator==(const ControllerState& x) const override;
    virtual bool is_neutral() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void execute(
        CancellableScope& scope,
        AbstractController& controller,
        Milliseconds duration
    ) const override;
    virtual std::string to_cpp(Milliseconds hold, Milliseconds release) const override;

public:
    Button buttons = BUTTON_NONE;
    uint8_t joystick_x = 128;
    uint8_t joystick_y = 128;
};




}
}
#endif
