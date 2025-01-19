/*  Overworld Movement
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldMovement_H
#define PokemonAutomation_PokemonSwSh_OverworldMovement_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_in_circle_up(SwitchControllerContext& context, bool counter_clockwise);
void move_in_circle_down(SwitchControllerContext& context, bool counter_clockwise);
void circle_in_place(SwitchControllerContext& context, bool counter_clockwise);
void move_in_line(SwitchControllerContext& context, bool horizontal);

}
}
}
#endif
