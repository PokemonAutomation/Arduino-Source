/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void IoA_backout(SwitchControllerContext& context, Milliseconds pokemon_to_menu_delay){
    //  Back out to menu.
    //  Prepend each B press by a DOWN press so that the B gets
    //  swallowed while in the summary.
    ssf_press_dpad(context, DPAD_DOWN, 5, 15);
    ssf_press_button(context, BUTTON_B, 60, 10);
    ssf_press_dpad(context, DPAD_DOWN, 5, 15);
    ssf_press_button(context, BUTTON_B, pokemon_to_menu_delay, 80ms);
}



}
}


