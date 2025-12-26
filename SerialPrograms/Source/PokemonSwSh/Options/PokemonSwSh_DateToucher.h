/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DateToucher_H
#define PokemonAutomation_DateToucher_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TimeRollbackHoursOption : public SimpleIntegerOption<uint8_t>{
public:
    TimeRollbackHoursOption();
};

class TouchDateIntervalOption : public SimpleIntegerOption<uint8_t>{
public:
    TouchDateIntervalOption();

    virtual void reset_state() override final;

    bool ok_to_touch_now();
    void touch_now_from_home_if_needed(ConsoleHandle& console, ProControllerContext& context);

private:
    SpinLock m_lock;
    WallClock m_last_touch;
};



}
}
}
#endif
