/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DateToucher_H
#define PokemonAutomation_DateToucher_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TimeRollbackHoursOption : public SimpleIntegerOption<uint8_t>{
public:
    TimeRollbackHoursOption();
};

#if 0
class TouchDateIntervalOptionTicks : public TimeExpressionOption<uint32_t>{
public:
    TouchDateIntervalOptionTicks()
        : TimeExpressionOption<uint32_t>(
            "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
            "4 * 3600 * TICKS_PER_SECOND"
        )
    {}
};
#endif

class TouchDateIntervalOption : public ConfigOption{
public:
    TouchDateIntervalOption();

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual QString check_validity() const override;

    virtual void restore_defaults() override;
    virtual void reset_state() override final;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

    bool ok_to_touch_now();
    void touch_now_from_home_if_needed(const BotBaseContext& context);

private:
    SimpleIntegerOption<uint8_t> m_hours;
    std::chrono::system_clock::time_point m_last_touch;
};



}
}
}
#endif
