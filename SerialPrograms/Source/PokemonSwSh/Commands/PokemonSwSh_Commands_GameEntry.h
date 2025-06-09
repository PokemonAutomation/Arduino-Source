/*  Game Entry Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_GameEntry_H
#define PokemonAutomation_PokemonSwSh_Commands_GameEntry_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void resume_game_no_interact_old        (ProControllerContext& device, bool tolerate_update_menu);
void resume_game_back_out_old           (ProControllerContext& device, bool tolerate_update_menu, uint16_t mash_B_time);
void resume_game_front_of_den_nowatts   (ProControllerContext& device, bool tolerate_update_menu);
void settings_to_enter_game             (ProControllerContext& device, bool fast);
void settings_to_enter_game_den_lobby(
    ProControllerContext& device,
    bool tolerate_update_menu, bool fast,
    Milliseconds enter_switch_pokemon_delay,
    Milliseconds exit_switch_pokemon_delay
);
void enter_game(
    ProControllerContext& device,
    bool backup_save,
    Milliseconds enter_game_mash,
    Milliseconds enter_game_wait
);
void start_game_from_home(
    ProControllerContext& device,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save
);
void fast_reset_game(
    ProControllerContext& device,
    Milliseconds start_game_mash,
    Milliseconds start_game_wait,
    Milliseconds enter_game_mash,
    Milliseconds enter_game_wait
);
void reset_game_from_home(
    ConsoleHandle& console, ProControllerContext& device,
    bool tolerate_update_menu
);




}
}
}
#endif
