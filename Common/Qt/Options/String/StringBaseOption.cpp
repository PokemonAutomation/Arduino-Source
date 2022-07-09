/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "StringBaseOption.h"

namespace PokemonAutomation{


StringBaseOption::StringBaseOption(
    bool is_password,
    QString label,
    QString default_value,
    QString placeholder_text
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
    , m_placeholder_text(std::move(placeholder_text))
    , m_is_password(is_password)
    , m_current(m_default)
{}
StringBaseOption::operator QString() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
QString StringBaseOption::get() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
void StringBaseOption::set(QString x){
    SpinLockGuard lg(m_lock);
    m_current = std::move(x);
}

void StringBaseOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}

void StringBaseOption::load_default(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_default = QString::fromStdString(*str);
}
void StringBaseOption::load_current(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = QString::fromStdString(*str);
}
JsonValue StringBaseOption::write_default() const{
    SpinLockGuard lg(m_lock);
    return m_default.toStdString();
}
JsonValue StringBaseOption::write_current() const{
    SpinLockGuard lg(m_lock);
    return m_current.toStdString();
}

    


}

