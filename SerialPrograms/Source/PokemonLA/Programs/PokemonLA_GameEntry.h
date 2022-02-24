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


bool gamemenu_to_ingame(
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t mash_duration, uint16_t enter_game_timeout
);
bool openedgame_to_ingame(
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t load_game_timeout,
    uint16_t mash_duration, uint16_t enter_game_timeout,
    uint16_t post_wait_time = 125
);
bool reset_game_from_home(
    ProgramEnvironment& env, ConsoleHandle& console,
    bool tolerate_update_menu,
    uint16_t post_wait_time = 125
);


bool save_game_from_overworld(ProgramEnvironment& env, ConsoleHandle& console);




}
}
}
#endif
