/*  Date Skippers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateSkippers_H
#define PokemonAutomation_NintendoSwitch_DateSkippers_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace DateSkippers{


void init_view(ProControllerContext& context);
void auto_recovery(ProControllerContext& context);
void increment_day(ProControllerContext& context, bool date_us);
void rollback_year_full(ProControllerContext& context, bool date_us);
void rollback_year_sync(ProControllerContext& context);
void increment_monthday(ProControllerContext& context);
void increment_daymonth(ProControllerContext& context);
void increment_month(ProControllerContext& context, uint8_t days);
void increment_all(ProControllerContext& context);
void increment_all_rollback(ProControllerContext& context);




}
}
}
#endif
