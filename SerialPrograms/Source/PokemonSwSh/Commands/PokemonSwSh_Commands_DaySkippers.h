/*  Day Skippers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H
#define PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//  Required Feature Flags:
//    - NintendoSwitch_DateSkip

void skipper_init_view              (ProControllerContext& context);
void skipper_auto_recovery          (ProControllerContext& context);
void skipper_rollback_year_full     (ProControllerContext& context, bool date_us);
void skipper_rollback_year_sync     (ProControllerContext& context);
void skipper_increment_day          (ProControllerContext& context, bool date_us);
void skipper_increment_month        (ProControllerContext& context, uint8_t days);
void skipper_increment_all          (ProControllerContext& context);
void skipper_increment_all_rollback (ProControllerContext& context);



}
}
#endif
