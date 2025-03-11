/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_EggRoutines_H
#define PokemonAutomation_PokemonSwSh_Commands_EggRoutines_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop        (ProControllerContext& context);
void move_while_mashing_B   (ProControllerContext& context, Milliseconds duration);
void spin_and_mash_A        (ProControllerContext& context, Milliseconds duration);
void travel_to_spin_location(ProControllerContext& context);
void travel_back_to_lady    (ProControllerContext& context);




}
}
#endif
