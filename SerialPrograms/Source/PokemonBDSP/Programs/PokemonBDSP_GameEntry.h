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


bool gamemenu_to_ingame(
    VideoStream& stream, SwitchControllerContext& context,
    uint16_t mash_duration, uint16_t enter_game_timeout
);
bool openedgame_to_ingame(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    uint16_t load_game_timeout,
    uint16_t mash_duration, uint16_t enter_game_timeout,
    uint16_t post_wait_time = 125
);
bool reset_game_from_home(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    bool tolerate_update_menu,
    uint16_t post_wait_time = 125
);



}
}
}
#endif
