/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/NintendoSwitch/NintendoSwitch_Tools.h"
#include "TimeExpressionBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
TimeExpressionBaseOption<Type>::TimeExpressionBaseOption(
    std::string label,
    Type min_value,
    Type max_value,
    std::string default_value
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
std::string TimeExpressionBaseOption<Type>::set(std::string text){
    Type value = 0;
    std::string error = process(text, value);
    if (error.empty()){
        SpinLockGuard lg(m_lock);
        m_current = std::move(text);
        m_value = value;
        m_error.clear();
    }
    return error;
}

template <typename Type>
std::string TimeExpressionBaseOption<Type>::text() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
template <typename Type>
std::string TimeExpressionBaseOption<Type>::time_string() const{
    SpinLockGuard lg(m_lock);
    if (!m_error.empty()){
        return "<font color=\"red\">" + m_error + "</font>";
    }
    return NintendoSwitch::ticks_to_time(m_value);
}

template <typename Type>
void TimeExpressionBaseOption<Type>::load_default(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    m_default = *str;
    m_error = process(m_current, m_value);
}
template <typename Type>
void TimeExpressionBaseOption<Type>::load_current(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    SpinLockGuard lg(m_lock);
    m_current = *str;
    m_error = process(m_current, m_value);
}
template <typename Type>
JsonValue TimeExpressionBaseOption<Type>::write_default() const{
    return m_default;
}
template <typename Type>
JsonValue TimeExpressionBaseOption<Type>::write_current() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

template <typename Type>
std::string TimeExpressionBaseOption<Type>::check_validity() const{
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
std::string TimeExpressionBaseOption<Type>::process(const std::string& text, Type& value) const{
    if (text.empty()){
        return "Expression is empty.";
    }
    int32_t parsed;
    try{
        parsed = parse_ticks_i32(text);
    }catch (const ParseException& str){
        return str.message();
    }
    // std::cout << "value = " << parsed << " " << m_min_value << " " << m_max_value << std::endl;

    if (parsed < (int64_t)m_min_value){
        return "Overflow: Number is too small.";
    }
    if (parsed > (int64_t)m_max_value){
        return "Overflow: Number is too large.";
    }
    value = (Type)parsed;
    return std::string();
}




template class TimeExpressionBaseOption<uint16_t>;
template class TimeExpressionBaseOption<uint32_t>;
template class TimeExpressionBaseOption<int16_t>;
template class TimeExpressionBaseOption<int32_t>;



}
}
