/*  Nintendo Switch Pro Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ProControllerState_H
#define PokemonAutomation_NintendoSwitch_ProControllerState_H

#include "Controllers/ControllerState.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class ProController;




class ProControllerState : public ControllerState{
public:
    using ControllerType = ProController;

public:
    virtual void clear() override;
    virtual bool operator==(const ControllerState& x) const override;
    virtual bool is_neutral() const override;

    virtual void load_json(const JsonObject& json) override;
    virtual JsonObject to_json() const override;

    virtual void execute(
        CancellableScope& scope,
        AbstractController& controller,
        Milliseconds duration
    ) const override;
    virtual std::string to_cpp(Milliseconds hold, Milliseconds release) const override;

public:
    Button buttons = BUTTON_NONE;
    DpadPosition dpad = DPAD_NONE;
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
};




}
}
#endif
