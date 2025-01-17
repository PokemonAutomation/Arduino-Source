/*  Game Entry Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_GameEntry_H
#define PokemonAutomation_PokemonSwSh_Commands_GameEntry_H

#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact_old        (ControllerContext& device, bool tolerate_update_menu);
void resume_game_back_out_old           (ControllerContext& device, bool tolerate_update_menu, uint16_t mash_B_time);
void resume_game_front_of_den_nowatts   (ControllerContext& device, bool tolerate_update_menu);
void settings_to_enter_game             (ControllerContext& device, bool fast);
void settings_to_enter_game_den_lobby   (
    ControllerContext& device,
    bool tolerate_update_menu, bool fast,
    uint16_t enter_switch_pokemon_delay,
    uint16_t exit_switch_pokemon_delay
);
void enter_game                         (ControllerContext& device, bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait);
void start_game_from_home               (ControllerContext& device, bool tolerate_update_menu, uint8_t game_slot, uint8_t user_slot, bool backup_save);
void fast_reset_game(
    ControllerContext& device,
    uint16_t start_game_mash, uint16_t start_game_wait,
    uint16_t enter_game_mash, uint16_t enter_game_wait
);
void reset_game_from_home               (ConsoleHandle& console, ControllerContext& device, bool tolerate_update_menu);




}
}
}
#endif
