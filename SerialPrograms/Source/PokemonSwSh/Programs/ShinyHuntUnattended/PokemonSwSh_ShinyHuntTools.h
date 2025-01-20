/*  Shiny Hunting Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H
#define PokemonAutomation_PokemonSwSh_ShinyHuntingTools_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void run_away_with_lights(SwitchControllerContext& context);
void enter_summary(SwitchControllerContext& context, bool regi_move_right);
void close_game_if_overworld(
    VideoStream& stream, SwitchControllerContext& context,
    bool touch_date,
    uint8_t rollback_hours
);



}
}
}
#endif
