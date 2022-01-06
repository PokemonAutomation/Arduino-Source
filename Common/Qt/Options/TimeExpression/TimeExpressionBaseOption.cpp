/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/NintendoSwitch/NintendoSwitch_Tools.h"
#include "TimeExpressionBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
TimeExpressionBaseOption<Type>::TimeExpressionBaseOption(
    QString label,
    Type min_value,
    Type max_value,
    QString default_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(std::move(default_value))
    , m_current(m_default)
{
    m_error = process(m_current, m_value);
}

template <typename Type>
TimeExpressionBaseOption<Type>::operator Type() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
Type TimeExpressionBaseOption<Type>::get() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
QString TimeExpressionBaseOption<Type>::set(QString text){
    Type value = 0;
    QString error = process(text, value);
    if (error.isEmpty()){
        SpinLockGuard lg(m_lock);
        m_current = std::move(text);
        m_value = value;
        m_error.clear();
    }
    return error;
}

template <typename Type>
QString TimeExpressionBaseOption<Type>::text() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
template <typename Type>
QString TimeExpressionBaseOption<Type>::time_string() const{
    SpinLockGuard lg(m_lock);
    if (!m_error.isEmpty()){
        return "<font color=\"red\">" + m_error + "</font>";
    }
    return QString::fromStdString(NintendoSwitch::ticks_to_time(m_value));
}

template <typename Type>
void TimeExpressionBaseOption<Type>::load_default(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    m_default = json.toString();
    m_error = process(m_current, m_value);
}
template <typename Type>
void TimeExpressionBaseOption<Type>::load_current(const QJsonValue& json){
    if (!json.isString()){
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = json.toString();
    m_error = process(m_current, m_value);
}
template <typename Type>
QJsonValue TimeExpressionBaseOption<Type>::write_default() const{
    return QJsonValue(m_default);
}
template <typename Type>
QJsonValue TimeExpressionBaseOption<Type>::write_current() const{
    SpinLockGuard lg(m_lock);
    return QJsonValue(m_current);
}

template <typename Type>
QString TimeExpressionBaseOption<Type>::check_validity() const{
    SpinLockGuard lg(m_lock);
    return m_error;
}
template <typename Type>
void TimeExpressionBaseOption<Type>::restore_defaults(){
    SpinLockGuard lg(m_lock);
    m_current = m_default;
    m_error = process(m_current, m_value);
}


template <typename Type>
QString TimeExpressionBaseOption<Type>::process(const QString& text, Type& value) const{
    if (text.isEmpty() || text.isNull()){
        return "Expression is empty.";
    }
    uint32_t parsed;
    try{
        parsed = parse_ticks_i32(text);
    }catch (const ParseException& str){
        return str.message_qt();
    }
//    cout << "value = " << parsed << endl;

    if (parsed < m_min_value){
        return "Overflow: Number is too small.";
    }
    if (parsed > m_max_value){
        return "Overflow: Number is too large.";
    }
    value = (Type)parsed;
    return QString();
}




template class TimeExpressionBaseOption<uint16_t>;
template class TimeExpressionBaseOption<uint32_t>;



}
}
