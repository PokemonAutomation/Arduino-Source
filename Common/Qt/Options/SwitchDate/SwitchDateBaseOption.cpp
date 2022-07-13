/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "SwitchDateBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



bool valid_switch_date(const QDate& date){
    if (date < QDate(2000, 1, 1)){
        return false;
    }
    if (date > QDate(2060, 12, 31)){
        return false;
    }
    return true;
}
bool json_parse_date(QDate& date, const JsonValue& value){
    const JsonArray* array = value.get_array();
    if (array == nullptr || array->size() != 3){
        return false;
    }
    int year, month, day;
    if (!(*array)[0].read_integer(year)){
        return false;
    }
    if (!(*array)[1].read_integer(month)){
        return false;
    }
    if (!(*array)[2].read_integer(day)){
        return false;
    }
    QDate try_date(year, month, day);
    if (!try_date.isValid() || !valid_switch_date(try_date)){
        return false;
    }
    date = try_date;
    return true;
}
JsonArray json_write_date(const QDate& date){
    JsonArray array;
    array.push_back(date.year());
    array.push_back(date.month());
    array.push_back(date.day());
    return array;
}


SwitchDateBaseOption::SwitchDateBaseOption(
    std::string label,
    QDate default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}

SwitchDateBaseOption::operator QDate() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QDate SwitchDateBaseOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
std::string SwitchDateBaseOption::set(QDate x){
    std::string err = check_validity(x);
    if (err.empty()){
        SpinLockGuard lg(m_lock);
        m_current = x;
    }
    return err;
}

std::string SwitchDateBaseOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
std::string SwitchDateBaseOption::check_validity(QDate x) const{
    return valid_switch_date(x) ? std::string() : "Invalid Switch date.";
}
void SwitchDateBaseOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}



void SwitchDateBaseOption::load_default(const JsonValue& json){
    json_parse_date(m_default, json);
}
void SwitchDateBaseOption::load_current(const JsonValue& json){
    SpinLockGuard lg(m_lock);
    json_parse_date(m_current, json);
}
JsonValue SwitchDateBaseOption::write_default() const{
    return json_write_date(m_default);
}
JsonValue SwitchDateBaseOption::write_current() const{
    SpinLockGuard lg(m_lock);
    return json_write_date(m_current);
}

    

}
}

