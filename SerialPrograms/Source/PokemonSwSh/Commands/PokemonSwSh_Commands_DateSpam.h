/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Commands_DateSpam_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ConsoleHandle;


constexpr uint8_t MAX_YEAR = 60;


void home_to_date_time                      (ControllerContext& context, bool to_date_change, bool fast);

namespace PokemonSwSh{

void neutral_date_skip                      (ControllerContext& context);
void roll_date_forward_1                    (ControllerContext& context, bool fast);
void roll_date_backward_N                   (ControllerContext& context, uint8_t skips, bool fast);
void home_roll_date_enter_game              (ConsoleHandle& console, ControllerContext& context, bool rollback_year);
void home_roll_date_enter_game_autorollback (ConsoleHandle& console, ControllerContext& context, uint8_t& year);
void touch_date_from_home                   (ControllerContext& context, uint16_t settings_to_home_delay);
void rollback_hours_from_home               (ControllerContext& context, uint8_t hours, uint16_t settings_to_home_delay);

}

}
}
#endif
