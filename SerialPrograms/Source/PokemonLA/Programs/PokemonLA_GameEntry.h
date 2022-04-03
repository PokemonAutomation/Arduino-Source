/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_GameEntry_H
#define PokemonAutomation_PokemonLA_GameEntry_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// From the game menu screen (where "Press A" is displayed to enter the game),
// mash A to enter the game and wait until the black screen is gone.
bool gamemenu_to_ingame(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    uint16_t mash_duration, uint16_t enter_game_timeout
);

// From Switch Home menu, start game and wait until the game menu screen (where 
// "Press A" is displayed to enter the game) is shown.
bool switch_home_to_gamemenu(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu
);

// From Switch Home menu, start game and wait until the player character appears in game.
// post_wait_time: how many ticks to wait after the black screen (shown when loading the map) is over.
bool reset_game_from_home(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    uint16_t post_wait_time = 125
);


void save_game_from_overworld(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);




}
}
}
#endif
