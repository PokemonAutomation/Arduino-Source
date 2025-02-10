/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Commands_DateSpam_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


constexpr uint8_t MAX_YEAR = 60;


void home_to_date_time                      (ProControllerContext& context, bool to_date_change, bool fast);

namespace PokemonSwSh{

void neutral_date_skip                      (ProControllerContext& context);
void roll_date_forward_1                    (ProControllerContext& context, bool fast);
void roll_date_backward_N                   (ProControllerContext& context, uint8_t skips, bool fast);
void home_roll_date_enter_game              (VideoStream& stream, ProControllerContext& context, bool rollback_year);
void home_roll_date_enter_game_autorollback (VideoStream& stream, ProControllerContext& context, uint8_t& year);
void touch_date_from_home                   (ProControllerContext& context, Milliseconds settings_to_home_delay);
void rollback_hours_from_home(
    ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
);

}

}
}
#endif
