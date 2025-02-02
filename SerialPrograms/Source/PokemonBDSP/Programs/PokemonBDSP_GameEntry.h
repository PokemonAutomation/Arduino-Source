/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameEntry_H
#define PokemonAutomation_PokemonBDSP_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


bool gamemenu_to_ingame(
    VideoStream& stream, SwitchControllerContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
);
bool openedgame_to_ingame(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    Milliseconds load_game_timeout,
    Milliseconds mash_duration, Milliseconds enter_game_timeout,
    Milliseconds post_wait_time = 1000ms
);
bool reset_game_from_home(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    bool tolerate_update_menu,
    Milliseconds post_wait_time = 1000ms
);



}
}
}
#endif
