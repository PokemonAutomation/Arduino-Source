/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_GameEntry_H
#define PokemonAutomation_PokemonSV_GameEntry_H

#include <stdint.h>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSV{



// From Switch Home menu, reset game and wait until the game menu screen (where
// "Press A" is displayed to enter the game) is shown.
bool reset_game_to_gamemenu(ConsoleHandle& console, ProControllerContext& context);

// From the game menu screen (where "Press A" is displayed to enter the game),
// mash A to enter the game and wait until the black screen is gone.
bool gamemenu_to_ingame(VideoStream& stream, ProControllerContext& context);

// From Switch Home menu, start game and wait until the player character appears in game.
// post_wait_time: how many ticks to wait after the black screen (shown when loading the map) is over.
// Return whether it successfully enters the game
bool reset_game_from_home(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    uint16_t post_wait_time = 125
);
bool reset_game_from_home_zoom_out(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    uint16_t post_wait_time = 125
);

// From within the game, reset the game.
// Will throw FatalProgramException if it fails to enter the game.
void reset_game(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context
);






}
}
}
#endif
