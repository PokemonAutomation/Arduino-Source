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
#include "TimeExpressionOption.h"

namespace PokemonAutomation{



std::string ticks_to_time(double ticks_per_second, int64_t ticks){
    const double SECOND = ticks_per_second;
    const double MINUTE = SECOND * 60;
    const double HOUR = MINUTE * 60;
    const double DAY = HOUR * 24;

    std::string str;
    str += tostr_u_commas(ticks);
    str += " tick";
    // Compute absolute value of the ticks:
    uint64_t abs_ticks = 0;
    // int64_t range from [-2^63, 2^63-1], so if ticks is -2^63, abs(ticks) will overflow.
    // This if-statement handles this case.
    if (ticks == std::numeric_limits<int64_t>::min()){
        abs_ticks = uint64_t(std::numeric_limits<int64_t>::max()) + 1;
    } else{
        abs_ticks = std::abs(ticks);
    }
    if (abs_ticks != 1){
        str += "s";
    }
    str += " (";
    if (ticks < 0){
        str += "-";
    }
    if (abs_ticks < MINUTE * 2){
        str += tostr_fixed((double)abs_ticks / SECOND, 3);
        str += " seconds";
    }else if (abs_ticks < HOUR * 2){
        str += tostr_fixed((double)abs_ticks / MINUTE, 3);
        str += " minutes";
    }else if (abs_ticks < DAY * 2){
        str += tostr_fixed((double)abs_ticks / HOUR, 3);
        str += " hours";
    }else{
        str += tostr_fixed((double)abs_ticks / DAY, 3);
        str += " days";
    }
    str += ")";
    return str;
}





template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    double ticks_per_second,
    std::string label,
    std::string default_value,
    Type min_value,
    Type max_value
)
    : m_label(std::move(label))
    , m_ticks_per_second(ticks_per_second)
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
    return ticks_to_time(m_ticks_per_second, m_value);
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
