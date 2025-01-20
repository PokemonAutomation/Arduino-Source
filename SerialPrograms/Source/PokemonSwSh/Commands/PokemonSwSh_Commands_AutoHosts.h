/*  Auto Host Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_AutoHosts_H
#define PokemonAutomation_PokemonSwSh_Commands_AutoHosts_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void connect_to_internet(
    SwitchControllerContext& context,
    uint16_t open_ycomm_delay,
    uint16_t connect_to_internet_delay
);
void home_to_add_friends(
    SwitchControllerContext& context,
    uint8_t user_slot,
    uint8_t scroll_down,
    bool fix_cursor
);
void accept_FRs(
    VideoStream& stream, SwitchControllerContext& context,
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
);




}
}
}
#endif
