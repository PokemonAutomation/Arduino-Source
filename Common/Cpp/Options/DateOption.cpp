/*  Date
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
//#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "DateOption.h"

namespace PokemonAutomation{





bool DateTimeCell::is_valid(const DateTime& date) const{
    if (date.year < 0){
        return false;
    }
    if (date.month < 0){
        return false;
    }
    if (date.day < 0){
        return false;
    }

    if (m_level < DATE_HOUR_MIN){
        return true;
    }

    if (date.hour < 0){
        return false;
    }
    if (date.minute < 0){
        return false;
    }

    if (m_level < DATE_HOUR_MIN_SEC){
        return true;
    }

    if (date.second < 0){
        return false;
    }

    return true;
}

DateTimeCell::DateTimeCell(
    LockMode lock_while_running,
    Level level,
    const DateTime& min_value, const DateTime& max_value,
    const DateTime& default_value
)
    : ConfigOption(lock_while_running)
    , m_level(level)
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{
    if (!is_valid(min_value)){
        throw InternalProgramError(nullptr, "DateTimeCell()", "Date is invalid.");
    }
    if (!is_valid(max_value)){
        throw InternalProgramError(nullptr, "DateTimeCell()", "Date is invalid.");
    }
    if (!is_valid(default_value)){
        throw InternalProgramError(nullptr, "DateTimeCell()", "Date is invalid.");
    }
}

DateTimeCell::operator DateTime() const{
    ReadSpinLock lg(m_lock);
    return m_current;
}
DateTime DateTimeCell::get() const{
    ReadSpinLock lg(m_lock);
    return m_current;
}
std::string DateTimeCell::set(const DateTime& x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    {
        WriteSpinLock lg(m_lock);
        if (x == m_current){
            return std::string();
        }
        m_current = x;
    }
    report_value_changed(this);
    return std::string();
}

std::string DateTimeCell::check_validity(const DateTime& x) const{
    if (x < m_min_value || x > m_max_value){
        return "Invalid date.";
    }
    return std::string();
}
std::string DateTimeCell::check_validity() const{
    ReadSpinLock lg(m_lock);
    return check_validity(m_current);
}
void DateTimeCell::restore_defaults(){
    set(m_default);
}

DateTime DateTimeCell::from_json(const JsonValue& json){
    DateTime ret;

    const JsonObject* object = json.to_object();
    if (object == nullptr){
        return ret;
    }

    object->read_integer(ret.year, "Year", 0, 9999);
    object->read_integer(ret.month, "Month", 1, 12);
    object->read_integer(ret.day, "Day", 1, 31);
    object->read_integer(ret.hour, "Hour", 0, 23);
    object->read_integer(ret.minute, "Minute", 0, 59);
    object->read_integer(ret.second, "Second", 0, 59);

    return ret;
}
JsonValue DateTimeCell::to_json(const DateTime& date){
    JsonObject ret;
    if (date.year   >= 0) ret["Year"  ] = date.year;
    if (date.month  >= 0) ret["Month" ] = date.month;
    if (date.day    >= 0) ret["Day"   ] = date.day;
    if (date.hour   >= 0) ret["Hour"  ] = date.hour;
    if (date.minute >= 0) ret["Minute"] = date.minute;
    if (date.second >= 0) ret["Second"] = date.second;
    return ret;
}
void DateTimeCell::load_json(const JsonValue& json){
    DateTime date = from_json(json);
    if (!is_valid(date) || !check_validity(date).empty()){
        return;
    }
    {
        WriteSpinLock lg(m_lock);
        m_current = date;
    }
    report_value_changed(this);
}
JsonValue DateTimeCell::to_json() const{
    DateTime current;
    {
        ReadSpinLock lg(m_lock);
        current = m_current;
    }
    return to_json(current);
}




DateTimeOption::DateTimeOption(
    std::string label,
    LockMode lock_while_running,
    Level level,
    const DateTime& min_value, const DateTime& max_value,
    const DateTime& default_value
)
    : DateTimeCell(lock_while_running, level, min_value, max_value, default_value)
    , m_label(std::move(label))
{}
















}
