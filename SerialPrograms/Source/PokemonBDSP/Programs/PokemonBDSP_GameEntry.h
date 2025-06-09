/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameEntry_H
#define PokemonAutomation_PokemonBDSP_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


bool gamemenu_to_ingame(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
);
bool openedgame_to_ingame(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    Milliseconds load_game_timeout,
    Milliseconds mash_duration, Milliseconds enter_game_timeout,
    Milliseconds post_wait_time = 1000ms
);
bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    Milliseconds post_wait_time = 1000ms
);



}
}
}
#endif
