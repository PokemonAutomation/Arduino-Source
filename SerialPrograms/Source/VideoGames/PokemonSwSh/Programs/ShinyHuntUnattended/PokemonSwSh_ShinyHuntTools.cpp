/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_ShinyHuntTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_summary(ProControllerContext& context, bool regi_move_right){
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 0ms);
    pbf_press_button(context, BUTTON_A, 80ms, 2000ms);
    pbf_press_button(context, BUTTON_A, 80ms, 1600ms);
    if (regi_move_right){
        pbf_move_left_joystick(context, {+1, 0}, 160ms, 240ms);
    }
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 0ms);
    pbf_press_button(context, BUTTON_A, 80ms, 80ms);    //  For Regi, this clears the dialog after running.
}


}
}
}

