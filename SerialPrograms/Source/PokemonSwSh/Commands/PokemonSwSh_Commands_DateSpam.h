/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Commands_DateSpam_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


constexpr uint8_t MAX_YEAR = 60;


void home_to_date_time                      (BotBaseContext& context, bool to_date_change, bool fast);
void neutral_date_skip                      (BotBaseContext& context);
void roll_date_forward_1                    (BotBaseContext& context, bool fast);
void roll_date_backward_N                   (BotBaseContext& context, uint8_t skips, bool fast);
void home_roll_date_enter_game              (BotBaseContext& context, bool rollback_year);
void home_roll_date_enter_game_autorollback (BotBaseContext& context, uint8_t* year);
void touch_date_from_home                   (BotBaseContext& context, uint16_t settings_to_home_delay);
void rollback_hours_from_home               (BotBaseContext& context, uint8_t hours, uint16_t settings_to_home_delay);



}
}
#endif
