/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void IoA_backout(ProControllerContext& context, Milliseconds pokemon_to_menu_delay){
    //  Back out to menu.
    //  Prepend each B press by a DOWN press so that the B gets
    //  swallowed while in the summary.
    ssf_press_dpad(context, DPAD_DOWN, 40ms, 120ms);
    ssf_press_button(context, BUTTON_B, 480ms, 80ms);
    ssf_press_dpad(context, DPAD_DOWN, 40ms, 120ms);
    ssf_press_button(context, BUTTON_B, pokemon_to_menu_delay, 80ms);
}



}
}


