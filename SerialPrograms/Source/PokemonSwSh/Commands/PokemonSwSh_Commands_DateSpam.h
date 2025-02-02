/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Commands_DateSpam_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


constexpr uint8_t MAX_YEAR = 60;


void home_to_date_time                      (SwitchControllerContext& context, bool to_date_change, bool fast);

namespace PokemonSwSh{

void neutral_date_skip                      (SwitchControllerContext& context);
void roll_date_forward_1                    (SwitchControllerContext& context, bool fast);
void roll_date_backward_N                   (SwitchControllerContext& context, uint8_t skips, bool fast);
void home_roll_date_enter_game              (VideoStream& stream, SwitchControllerContext& context, bool rollback_year);
void home_roll_date_enter_game_autorollback (VideoStream& stream, SwitchControllerContext& context, uint8_t& year);
void touch_date_from_home                   (SwitchControllerContext& context, Milliseconds settings_to_home_delay);
void rollback_hours_from_home(
    SwitchControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
);

}

}
}
#endif
