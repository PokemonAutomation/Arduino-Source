/*  Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "DateOption.h"

namespace PokemonAutomation{



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
    push_update();
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
    push_update();
}
JsonValue DateOption::to_json() const{
    return to_json(m_current);
}

std::string DateOption::check_validity(QDate x) const{
    if (x < m_min_value || x > m_max_value){
        return "Invalid Switch date.";
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
    push_update();
}





}
