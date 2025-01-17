/*  Misc. Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_Misc_H
#define PokemonAutomation_PokemonSwSh_Commands_Misc_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//void mash_A         (ControllerContext& context, uint16_t ticks);
void IoA_backout    (ControllerContext& context, uint16_t pokemon_to_menu_delay);



}
}
#endif
