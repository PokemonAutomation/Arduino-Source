/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DateToucher_H
#define PokemonAutomation_DateToucher_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TimeRollbackHours : public SimpleInteger<uint8_t>{
public:
    TimeRollbackHours()
        : SimpleInteger<uint8_t>(
            "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
            1, 0, 11
        )
    {}
};

class TouchDateInterval : public TimeExpression<uint32_t>{
public:
    TouchDateInterval()
        : TimeExpression<uint32_t>(
            "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
            "4 * 3600 * TICKS_PER_SECOND"
        )
    {}
};


}
}
}
#endif
