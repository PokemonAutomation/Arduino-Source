/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void resume_game_from_home(
    VideoStream& stream, SwitchControllerContext& context,
    bool skip_home_press = false
);


void start_game_from_home(
    VideoStream& stream, SwitchControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
);

bool openedgame_to_gamemenu(
    VideoStream& stream, SwitchControllerContext& context,
    Milliseconds timeout
);



}
}
#endif
