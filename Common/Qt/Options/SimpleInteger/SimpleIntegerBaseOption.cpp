/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "SimpleIntegerBaseOption.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerBaseOption<Type>::SimpleIntegerBaseOption(
    QString label,
    Type min_value,
    Type max_value,
    Type default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
    , m_current(default_value)
{}

template <typename Type>
QString SimpleIntegerBaseOption<Type>::set(Type x){
    QString err = check_validity(x);
    if (err.isEmpty()){
        m_current.store(x, std::memory_order_relaxed);
    }
    return err;
}

template <typename Type>
QString SimpleIntegerBaseOption<Type>::check_validity(Type x) const{
    if (x < m_min_value){
        return "Value too small: min = " + QString::number(m_min_value) + ", value = " + QString::number(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + QString::number(m_max_value) + ", value = " + QString::number(x);
    }
    return QString();
}
template <typename Type>
void SimpleIntegerBaseOption<Type>::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}

template <typename Type>
void SimpleIntegerBaseOption<Type>::load_default(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    m_default = json.toInt();
    m_default = std::max(m_default, m_min_value);
    m_default = std::min(m_default, m_max_value);
}
template <typename Type>
void SimpleIntegerBaseOption<Type>::load_current(const QJsonValue& json){
    if (!json.isDouble()){
        return;
    }
    Type current = json.toInt();
    current = std::max(current, m_min_value);
    current = std::min(current, m_max_value);
    m_current.store(current, std::memory_order_relaxed);
}
template <typename Type>
QJsonValue SimpleIntegerBaseOption<Type>::write_default() const{
    return QJsonValue((qint64)m_default);
}
template <typename Type>
QJsonValue SimpleIntegerBaseOption<Type>::write_current() const{
    return QJsonValue((qint64)m_current);
}

template <typename Type>
QString SimpleIntegerBaseOption<Type>::check_validity() const{
    Type current = m_current.load(std::memory_order_relaxed);
    return check_validity(current);
}





template class SimpleIntegerBaseOption<uint8_t>;
template class SimpleIntegerBaseOption<uint16_t>;
template class SimpleIntegerBaseOption<uint32_t>;
template class SimpleIntegerBaseOption<uint64_t>;
template class SimpleIntegerBaseOption<int8_t>;



}
