/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_GameEntry_H
#define PokemonAutomation_PokemonSV_GameEntry_H

#include <stdint.h>

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSV{



// From Switch Home menu, reset game and wait until the game menu screen (where
// "Press A" is displayed to enter the game) is shown.
bool reset_game_to_gamemenu(ConsoleHandle& console, BotBaseContext& context);

// From the game menu screen (where "Press A" is displayed to enter the game),
// mash A to enter the game and wait until the black screen is gone.
bool gamemenu_to_ingame(ConsoleHandle& console, BotBaseContext& context);

// From Switch Home menu, start game and wait until the player character appears in game.
// post_wait_time: how many ticks to wait after the black screen (shown when loading the map) is over.
// Return whether it successfully enters the game
bool reset_game_from_home(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint16_t post_wait_time = 125
);





}
}
}
#endif
