/*  Day Skippers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H
#define PokemonAutomation_PokemonSwSh_Commands_DaySkippers_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//  Required Feature Flags:
//    - NintendoSwitch_DateSkip

void skipper_init_view              (SwitchControllerContext& context);
void skipper_auto_recovery          (SwitchControllerContext& context);
void skipper_rollback_year_full     (SwitchControllerContext& context, bool date_us);
void skipper_rollback_year_sync     (SwitchControllerContext& context);
void skipper_increment_day          (SwitchControllerContext& context, bool date_us);
void skipper_increment_month        (SwitchControllerContext& context, uint8_t days);
void skipper_increment_all          (SwitchControllerContext& context);
void skipper_increment_all_rollback (SwitchControllerContext& context);



}
}
#endif
