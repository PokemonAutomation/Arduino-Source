/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void open_game_from_home(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_mash
);

bool openedgame_to_gamemenu(
    ProgramEnvironment& env, const BotBaseContext& context, ConsoleHandle& console,
    uint16_t timeout
);



}
}
#endif
