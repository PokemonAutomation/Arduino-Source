/*  Auto Host Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_AutoHosts_H
#define PokemonAutomation_PokemonSwSh_Commands_AutoHosts_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void connect_to_internet(
    ProControllerContext& context,
    Milliseconds open_ycomm_delay,
    Milliseconds connect_to_internet_delay
);
void home_to_add_friends(
    ProControllerContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
);
void accept_FRs(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t slot, bool fix_cursor,
    Milliseconds game_to_home_delay_safe,
    Milliseconds auto_fr_duration,
    bool tolerate_system_update_window_slow
);




}
}
}
#endif
