/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include <QJsonValue>
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

void StringBaseOption::load_default(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_default = json.toString();
}
void StringBaseOption::load_current(const QJsonValue& json){
    if (!json.isString()) {
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
}
QJsonValue StringBaseOption::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue StringBaseOption::write_current() const{
    SpinLockGuard lg(m_lock);
    return QJsonValue(m_current);
}

    


}

