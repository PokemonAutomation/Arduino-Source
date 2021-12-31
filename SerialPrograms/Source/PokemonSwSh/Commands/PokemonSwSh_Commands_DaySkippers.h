/*  Day Skippers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H
#define PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void skipper_init_view              (const BotBaseContext& context);
void skipper_auto_recovery          (const BotBaseContext& context);
void skipper_rollback_year_full     (const BotBaseContext& context, bool date_us);
void skipper_rollback_year_sync     (const BotBaseContext& context);
void skipper_increment_day          (const BotBaseContext& context, bool date_us);
void skipper_increment_month        (const BotBaseContext& context, uint8_t days);
void skipper_increment_all          (const BotBaseContext& context);
void skipper_increment_all_rollback (const BotBaseContext& context);



}
}
#endif
