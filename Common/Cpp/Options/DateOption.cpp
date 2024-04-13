/*  Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
//#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "DateOption.h"

namespace PokemonAutomation{



#if 0
DateOption::DateOption(
    std::string label,
    QDate min_value, QDate max_value,
    QDate default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}
#if 0
std::unique_ptr<ConfigOption> DateOption::clone() const{
    std::unique_ptr<DateOption> ret(new DateOption(m_label, m_min_value, m_max_value, m_default));
    ret->m_current = m_current;
    return ret;
}
#endif


DateOption::operator QDate() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QDate DateOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
std::string DateOption::set(QDate x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = x;
    }
    report_value_changed();
    return std::string();
}

QDate DateOption::from_json(const JsonValue& json){
    const JsonArray* array = json.get_array();
    if (array == nullptr || array->size() != 3){
        return QDate();
    }
    int year, month, day;
    if (!(*array)[0].read_integer(year)){
        return QDate();
    }
    if (!(*array)[1].read_integer(month)){
        return QDate();
    }
    if (!(*array)[2].read_integer(day)){
        return QDate();
    }
    QDate date(year, month, day);
    if (!date.isValid()){
        return QDate();
    }
    return date;
}
JsonValue DateOption::to_json(const QDate& date){
    JsonArray array;
    array.push_back(date.year());
    array.push_back(date.month());
    array.push_back(date.day());
    return array;
}
void DateOption::load_json(const JsonValue& json){
    QDate date = from_json(json);
    if (!date.isValid() || !check_validity(date).empty()){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = date;
    }
    report_value_changed();
}
JsonValue DateOption::to_json() const{
    QDate date;
    {
        SpinLockGuard lg(m_lock);
        date = m_current;
    }
    return to_json(date);
}

std::string DateOption::check_validity(QDate x) const{
    if (x < m_min_value || x > m_max_value){
        return "Invalid date.";
    }
    return std::string();
}
std::string DateOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
void DateOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    report_value_changed();
}
#endif




bool DateTimeOption::is_valid(const DateTime& date) const{
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

DateTimeOption::DateTimeOption(
    std::string label,
    LockMode lock_while_running,
    Level level,
    const DateTime& min_value, const DateTime& max_value,
    const DateTime& default_value
)
    : ConfigOption(lock_while_running)
    , m_label(std::move(label))
    , m_level(level)
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{
    if (!is_valid(min_value)){
        throw InternalProgramError(nullptr, "DateTimeOption()", "Date is invalid.");
    }
    if (!is_valid(max_value)){
        throw InternalProgramError(nullptr, "DateTimeOption()", "Date is invalid.");
    }
    if (!is_valid(default_value)){
        throw InternalProgramError(nullptr, "DateTimeOption()", "Date is invalid.");
    }
}

DateTimeOption::operator DateTime() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
DateTime DateTimeOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
std::string DateTimeOption::set(const DateTime& x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = x;
    }
    report_value_changed();
    return std::string();
}

std::string DateTimeOption::check_validity(const DateTime& x) const{
    if (x < m_min_value || x > m_max_value){
        return "Invalid date.";
    }
    return std::string();
}
std::string DateTimeOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
void DateTimeOption::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
    }
    report_value_changed();
}

DateTime DateTimeOption::from_json(const JsonValue& json){
    DateTime ret;

    const JsonObject* object = json.get_object();
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
JsonValue DateTimeOption::to_json(const DateTime& date){
    JsonObject ret;
    if (date.year   >= 0) ret["Year"  ] = date.year;
    if (date.month  >= 0) ret["Month" ] = date.month;
    if (date.day    >= 0) ret["Day"   ] = date.day;
    if (date.hour   >= 0) ret["Hour"  ] = date.hour;
    if (date.minute >= 0) ret["Minute"] = date.minute;
    if (date.second >= 0) ret["Second"] = date.second;
    return ret;
}
void DateTimeOption::load_json(const JsonValue& json){
    DateTime date = from_json(json);
    if (!is_valid(date) || !check_validity(date).empty()){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = date;
    }
    report_value_changed();
}
JsonValue DateTimeOption::to_json() const{
    DateTime current;
    {
        SpinLockGuard lg(m_lock);
        current = m_current;
    }
    return to_json(current);
}




















}
