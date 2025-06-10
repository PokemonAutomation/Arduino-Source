/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Commands_DateSpam_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


constexpr uint8_t MAX_YEAR = 60;


namespace PokemonSwSh{

//void neutral_date_skip                      (ProControllerContext& context);
//void roll_date_forward_1                    (ProControllerContext& context, bool fast);
//void roll_date_backward_N                   (ProControllerContext& context, uint8_t skips, bool fast);
//void home_roll_date_enter_game              (ConsoleHandle& console, ProControllerContext& context, bool rollback_year);
void home_roll_date_enter_game_autorollback (ConsoleHandle& console, ProControllerContext& context, uint8_t& year);
void touch_date_from_home                   (
    ConsoleHandle& console, ProControllerContext& context,
    Milliseconds settings_to_home_delay
);
void rollback_hours_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
);

}

}
}
#endif
