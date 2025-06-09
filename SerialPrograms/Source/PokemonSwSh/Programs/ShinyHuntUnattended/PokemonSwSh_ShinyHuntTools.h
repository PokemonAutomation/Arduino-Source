/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void run_away_with_lights(ProControllerContext& context);
void enter_summary(ProControllerContext& context, bool regi_move_right);
void close_game_if_overworld(
    ConsoleHandle& console, ProControllerContext& context,
    bool touch_date,
    uint8_t rollback_hours
);



}
}
}
#endif
