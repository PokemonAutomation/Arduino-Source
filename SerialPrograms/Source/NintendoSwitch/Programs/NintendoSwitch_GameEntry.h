/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GameEntry_H
#define PokemonAutomation_NintendoSwitch_GameEntry_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void resume_game_from_home(
    ConsoleHandle& console,
    ControllerContext& context,
    bool skip_home_press = false
);


void start_game_from_home(
    ConsoleHandle& console,
    ControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_mash
);

bool openedgame_to_gamemenu(
    ConsoleHandle& console, ControllerContext& context,
    uint16_t timeout
);



}
}
#endif
