/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DateToucher_H
#define PokemonAutomation_DateToucher_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TimeRollbackHoursOption : public SimpleIntegerOption<uint8_t>{
public:
    TimeRollbackHoursOption()
        : SimpleIntegerOption<uint8_t>(
            "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
            1, 0, 11
        )
    {}
};

class TouchDateIntervalOption : public TimeExpressionOption<uint32_t>{
public:
    TouchDateIntervalOption()
        : TimeExpressionOption<uint32_t>(
            "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
            "4 * 3600 * TICKS_PER_SECOND"
        )
    {}
};


}
}
}
#endif
