/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_GameEntry_H
#define PokemonAutomation_PokemonLGPE_GameEntry_H

#include <stdint.h>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLGPE{



// From Switch Home menu, reset game and wait until the game menu screen (where
// "Press A" is displayed to enter the game) is shown.
bool reset_game_to_gamemenu(
    ConsoleHandle& console, JoyconContext& context
);

// From the game menu screen (where "Press A" is displayed to enter the game),
// mash A to enter the game and wait until the black screen is gone.
bool gamemenu_to_ingame(
    VideoStream& stream, JoyconContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
);

// From Switch Home menu, start game and wait until the player character appears in game.
// post_wait_time: how many ticks to wait after the black screen (shown when loading the map) is over.
bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, JoyconContext& context,
    Milliseconds post_wait_time = std::chrono::milliseconds(125)
);

void reset_game_from_game(
    ProgramEnvironment& env,
    ConsoleHandle& console, JoyconContext& context,
    std::atomic<uint64_t>* errors,
    Milliseconds post_wait_time = std::chrono::milliseconds(125)
);




}
}
}
#endif
