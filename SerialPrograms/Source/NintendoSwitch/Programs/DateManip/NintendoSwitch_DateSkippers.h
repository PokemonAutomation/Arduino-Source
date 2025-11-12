/*  Date Skippers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateSkippers_H
#define PokemonAutomation_NintendoSwitch_DateSkippers_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "CommonFramework/Tools/VideoStream.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace DateSkippers{


namespace Switch1{
    void init_view(ProControllerContext& context);
    void auto_recovery(ProControllerContext& context);
    void increment_day_with_feedback(VideoStream& stream, ProControllerContext& context, bool date_us);
    void increment_day(ProControllerContext& context, bool date_us);
    void rollback_year_full(ProControllerContext& context, bool date_us);
    void rollback_year_sync(ProControllerContext& context);
    void increment_monthday(ProControllerContext& context);
    void increment_daymonth(ProControllerContext& context);
    void increment_month(ProControllerContext& context, uint8_t days);
    void increment_all(ProControllerContext& context);
    void increment_all_rollback(ProControllerContext& context);
}
namespace Switch2{
    void init_view(ProControllerContext& context);
    void increment_day_us(ProControllerContext& context);
    void increment_day_eu(ProControllerContext& context);
    void increment_day_jp(ProControllerContext& context);
}



}
}
}
#endif
