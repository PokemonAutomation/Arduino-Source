/*  Overworld Movement
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldMovement_H
#define PokemonAutomation_PokemonSwSh_OverworldMovement_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_in_circle_up(ProControllerContext& context, bool counter_clockwise);
void move_in_circle_down(ProControllerContext& context, bool counter_clockwise);
void circle_in_place(ProControllerContext& context, bool counter_clockwise);
void move_in_line(ProControllerContext& context, bool horizontal);

}
}
}
#endif
