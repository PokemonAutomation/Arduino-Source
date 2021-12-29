/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <QJsonValue>
#include "FloatingPointBaseOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


FloatingPointBaseOption::FloatingPointBaseOption(
    QString label,
    double min_value,
    double max_value,
    double default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

QString FloatingPointBaseOption::set(double x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        m_current.store(x, std::memory_order_relaxed);
    }
    return err;
}
void FloatingPointBaseOption::restore_defaults(){
    m_current = m_default;
}

void FloatingPointBaseOption::load_default(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    m_default = json.toDouble();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
void FloatingPointBaseOption::load_current(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    double current = json.toDouble();
    current = std::max(current, m_min_value);
    current = std::min(current, m_max_value);
    m_current.store(current, std::memory_order_relaxed);
}
QJsonValue FloatingPointBaseOption::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue FloatingPointBaseOption::write_current() const{
    return QJsonValue(m_current.load(std::memory_order_relaxed));
}

QString FloatingPointBaseOption::check_validity() const{
    return check_validity(m_current.load(std::memory_order_relaxed));
}
QString FloatingPointBaseOption::check_validity(double x) const{
    if (x < m_min_value){
        return "Value too small: min = " + QString::number(m_min_value) + ", value = " + QString::number(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + QString::number(m_max_value) + ", value = " + QString::number(x);
    }
    if (std::isnan(x)){
        return "Value is NaN";
    }
    return QString();
}




}
