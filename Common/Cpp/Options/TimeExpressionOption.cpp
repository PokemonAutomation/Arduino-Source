/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "Common/Qt/Options/TimeExpressionWidget.h"
#include "Common/NintendoSwitch/NintendoSwitch_Tools.h"
#include "TimeExpressionOption.h"

namespace PokemonAutomation{


template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    std::string label,
    std::string default_value,
    Type min_value,
    Type max_value
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
TimeExpressionOption<Type>::operator Type() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
Type TimeExpressionOption<Type>::get() const{
    SpinLockGuard lg(m_lock);
    return m_value;
}
template <typename Type>
std::string TimeExpressionOption<Type>::set(std::string text){
    Type value = 0;
    std::string error = process(text, value);
    {
        SpinLockGuard lg(m_lock);
        m_current = std::move(text);
        m_value = value;
        m_error.clear();
    }
    push_update();
    return error;
}
template <typename Type>
std::string TimeExpressionOption<Type>::text() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}
template <typename Type>
std::string TimeExpressionOption<Type>::time_string() const{
    SpinLockGuard lg(m_lock);
    if (!m_error.empty()){
        return "<font color=\"red\">" + m_error + "</font>";
    }
    return NintendoSwitch::ticks_to_time(m_value);
}

template <typename Type>
void TimeExpressionOption<Type>::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        m_current = *str;
        m_error = process(m_current, m_value);
    }
    push_update();
}
template <typename Type>
JsonValue TimeExpressionOption<Type>::to_json() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

template <typename Type>
std::string TimeExpressionOption<Type>::check_validity() const{
    SpinLockGuard lg(m_lock);
    return m_error;
}
template <typename Type>
void TimeExpressionOption<Type>::restore_defaults(){
    {
        SpinLockGuard lg(m_lock);
        m_current = m_default;
        m_error = process(m_current, m_value);
    }
    push_update();
}

template <typename Type>
std::string TimeExpressionOption<Type>::process(const std::string& text, Type& value) const{
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





template <typename Type>
ConfigWidget* TimeExpressionOption<Type>::make_ui(QWidget& parent){
    return new TimeExpressionWidget<Type>(parent, *this);
}


template class TimeExpressionOption<uint16_t>;
template class TimeExpressionOption<uint32_t>;
template class TimeExpressionOption<int16_t>;
template class TimeExpressionOption<int32_t>;



}
