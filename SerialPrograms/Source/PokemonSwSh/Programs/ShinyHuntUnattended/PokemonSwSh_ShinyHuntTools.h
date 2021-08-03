/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H

#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void run_away_with_lights(const BotBaseContext& context);
void enter_summary(const BotBaseContext& context, bool regi_move_right);
void close_game_if_overworld(const BotBaseContext& context, bool touch_date, uint8_t rollback_hours);



}
}
}
#endif
