/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#include <QJsonValue>
#include <QJsonArray>
#include "Common/Qt/QtJsonTools.h"
#include "SwitchDateBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SwitchDateBaseOption::SwitchDateBaseOption(
    QString label,
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
QString SwitchDateBaseOption::set(QDate x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        SpinLockGuard lg(m_lock);
        m_current = x;
    }
    return err;
}

QString SwitchDateBaseOption::check_validity() const{
    SpinLockGuard lg(m_lock);
    return check_validity(m_current);
}
QString SwitchDateBaseOption::check_validity(QDate x) const{
    return valid_switch_date(x) ? QString() : "Invalid Switch date.";
}
void SwitchDateBaseOption::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
}

void SwitchDateBaseOption::load_default(const QJsonValue& json){
    json_parse_date(m_default, json);
}
void SwitchDateBaseOption::load_current(const QJsonValue& json){
    SpinLockGuard lg(m_lock);
    json_parse_date(m_current, json);
}
QJsonValue SwitchDateBaseOption::write_default() const{
    return json_write_date(m_default);
}
QJsonValue SwitchDateBaseOption::write_current() const{
    SpinLockGuard lg(m_lock);
    return json_write_date(m_current);
}

    

}
}

