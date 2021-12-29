/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "BooleanCheckBoxBaseOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


BooleanCheckBoxBaseOption::BooleanCheckBoxBaseOption(
    QString label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}

void BooleanCheckBoxBaseOption::load_default(const QJsonValue& json){
    if (!json.isBool()){
        return;
    }
    m_default = json.toBool();
}
void BooleanCheckBoxBaseOption::load_current(const QJsonValue& json){
    if (!json.isBool()){
        return;
    }
    m_current.store(json.toBool(), std::memory_order_relaxed);
}
QJsonValue BooleanCheckBoxBaseOption::write_default() const{
    return QJsonValue(m_default);
}
QJsonValue BooleanCheckBoxBaseOption::write_current() const{
    return QJsonValue(m_current.load(std::memory_order_relaxed));
}

void BooleanCheckBoxBaseOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}





}

