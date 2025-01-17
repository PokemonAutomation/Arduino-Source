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


void skipper_init_view              (ControllerContext& context);
void skipper_auto_recovery          (ControllerContext& context);
void skipper_rollback_year_full     (ControllerContext& context, bool date_us);
void skipper_rollback_year_sync     (ControllerContext& context);
void skipper_increment_day          (ControllerContext& context, bool date_us);
void skipper_increment_month        (ControllerContext& context, uint8_t days);
void skipper_increment_all          (ControllerContext& context);
void skipper_increment_all_rollback (ControllerContext& context);



}
}
#endif
