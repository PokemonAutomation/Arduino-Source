/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_EggRoutines_H
#define PokemonAutomation_PokemonSwSh_Commands_EggRoutines_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop        (ControllerContext& context);
void move_while_mashing_B   (ControllerContext& context, uint16_t duration);
void spin_and_mash_A        (ControllerContext& context, uint16_t duration);
void travel_to_spin_location(ControllerContext& context);
void travel_back_to_lady    (ControllerContext& context);




}
}
#endif
