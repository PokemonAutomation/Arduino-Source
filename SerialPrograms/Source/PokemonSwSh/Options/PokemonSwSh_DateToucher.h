/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DateToucher_H
#define PokemonAutomation_DateToucher_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TimeRollbackHoursOption : public SimpleIntegerOption<uint8_t>{
public:
    TimeRollbackHoursOption();
};

class TouchDateIntervalOption : public ConfigOption{
public:
    TouchDateIntervalOption();

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual std::string check_validity() const override;

    virtual void restore_defaults() override;
    virtual void reset_state() override final;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

    bool ok_to_touch_now();
    void touch_now_from_home_if_needed(ConsoleHandle& console, ProControllerContext& context);

private:
    SimpleIntegerOption<uint8_t> m_hours;

    SpinLock m_lock;
    WallClock m_last_touch;
};



}
}
}
#endif
