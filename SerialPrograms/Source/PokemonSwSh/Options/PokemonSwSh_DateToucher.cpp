/*  Date Toucher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh_DateToucher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


TimeRollbackHoursOption::TimeRollbackHoursOption()
    : SimpleIntegerOption<uint8_t>(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 0, 11
    )
{}





TouchDateIntervalOption::TouchDateIntervalOption()
    : m_hours(
        "<b>Rollover Prevention:</b><br>Prevent a date-skip by touching the date every this many hours. If set to zero, this feature is disabled.",
        LockMode::LOCK_WHILE_RUNNING,
        4, 0, 11
    )
{
    reset_state();
}


void TouchDateIntervalOption::load_json(const JsonValue& json){
    m_hours.load_json(json);
}
JsonValue TouchDateIntervalOption::to_json() const{
    return m_hours.to_json();
}
std::string TouchDateIntervalOption::check_validity() const{
    return m_hours.check_validity();
}
void TouchDateIntervalOption::restore_defaults(){
    m_hours.restore_defaults();
}
void TouchDateIntervalOption::reset_state(){
    WriteSpinLock lg(m_lock);
    m_last_touch = WallClock::min();
}
ConfigWidget* TouchDateIntervalOption::make_QtWidget(QWidget& parent){
    return m_hours.make_QtWidget(parent);
}


bool TouchDateIntervalOption::ok_to_touch_now(){
    uint8_t hours = m_hours;
    if (hours == 0){
        return false;
    }
    auto now = current_time();
    WriteSpinLock lg(m_lock);
    if (now < m_last_touch + std::chrono::hours(hours)){
        return false;
    }
    m_last_touch = now;
    return true;
}
void TouchDateIntervalOption::touch_now_from_home_if_needed(ConsoleHandle& console, ProControllerContext& context){
    if (!ok_to_touch_now()){
        return;
    }
    touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
}



}
}
}

