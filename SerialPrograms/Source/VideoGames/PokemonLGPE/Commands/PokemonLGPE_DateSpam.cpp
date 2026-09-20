/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLGPE_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

void roll_date_forward_1(JoyconContext& context){
    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, {0, +1}, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_move_joystick(context, {0, +1}, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
}

void roll_date_backward_N(JoyconContext& context, uint8_t skips){
    if (skips == 0){
        return;
    }

    Milliseconds tv = context->timing_variation();
    Milliseconds unit = 24ms + tv;

    pbf_press_button(context, BUTTON_A, 2*unit, unit);

    for (uint8_t c = 0; c < skips - 1; c++){
        pbf_move_joystick(context, {0, -1}, 2*unit, unit);
    }

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);

    for (uint8_t c = 0; c < skips - 1; c++){
        pbf_move_joystick(context, {0, -1}, 2*unit, unit);
    }

    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_move_joystick(context, {+1, 0}, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
    pbf_press_button(context, BUTTON_A, 2*unit, unit);
}





}

}
}

