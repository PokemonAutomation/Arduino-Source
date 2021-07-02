/*  Overworld Movement
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldMovement_H
#define PokemonAutomation_PokemonSwSh_OverworldMovement_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void move_in_circle_up(const BotBaseContext& context, bool counter_clockwise);
void move_in_circle_down(const BotBaseContext& context, bool counter_clockwise);
void circle_in_place(const BotBaseContext& context, bool counter_clockwise);
void move_in_line(const BotBaseContext& context, bool horizontal);

}
}
}
#endif
