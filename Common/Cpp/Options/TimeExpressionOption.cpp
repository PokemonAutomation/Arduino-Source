/*  Time Expression Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ExpressionEvaluator.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
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
struct TimeExpressionCell<Type>::Data{
    const double m_ticks_per_second;
    const double m_milliseconds_per_tick;
    const Type m_min_value;
    const Type m_max_value;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
    Type m_value;
    std::string m_error;

    Data(
        double ticks_per_second, Type min_value, Type max_value,
        std::string default_value
    )
        : m_ticks_per_second(ticks_per_second)
        , m_milliseconds_per_tick(1000 / ticks_per_second)
        , m_min_value(min_value)
        , m_max_value(max_value)
        , m_default(std::move(default_value))
        , m_current(m_default)
        , m_value(0)
    {
        m_error = process(m_current, m_value);
    }

    std::string process(const std::string& text, Type& value) const{
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
};


template <typename Type>
TimeExpressionCell<Type>::~TimeExpressionCell() = default;
template <typename Type>
TimeExpressionCell<Type>::TimeExpressionCell(
    LockMode lock_while_running,
    double ticks_per_second,
    Type min_value, Type max_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        ticks_per_second,
        min_value, max_value,
        std::move(default_value)
    )
{}

template <typename Type>
TimeExpressionCell<Type>::TimeExpressionCell(
    LockMode lock_while_running,
    double ticks_per_second,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        ticks_per_second,
        std::numeric_limits<Type>::min(), std::numeric_limits<Type>::max(),
        std::move(default_value)
    )
{}
template <typename Type>
TimeExpressionCell<Type>::TimeExpressionCell(
    LockMode lock_while_running,
    double ticks_per_second,
    Type min_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        ticks_per_second,
        min_value, std::numeric_limits<Type>::max(),
        std::move(default_value)
    )
{}

template <typename Type>
double TimeExpressionCell<Type>::ticks_per_second() const{
    return m_data->m_ticks_per_second;
}
template <typename Type>
Type TimeExpressionCell<Type>::min_value() const{
    return m_data->m_min_value;
}
template <typename Type>
Type TimeExpressionCell<Type>::max_value() const{
    return m_data->m_max_value;
}
template <typename Type>
const std::string& TimeExpressionCell<Type>::default_value() const{
    return m_data->m_default;
}
template <typename Type>
std::string TimeExpressionCell<Type>::current_text() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_current;
}
template <typename Type>
TimeExpressionCell<Type>::operator Type() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_value;
}
#if 0
template <typename Type>
TimeExpressionCell<Type>::operator Milliseconds() const{
    Type value = (Type)*this;
    double millis = (double)value * m_data->m_milliseconds_per_tick;
    return Milliseconds(Milliseconds::rep(millis > 0 ? millis + 0.5 : millis - 0.5));
}
#endif
template <typename Type>
Type TimeExpressionCell<Type>::get() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_value;
}
template <typename Type>
std::string TimeExpressionCell<Type>::set(std::string text){
    Data& data = *m_data;
    Type value = 0;
    std::string error;
    {
        WriteSpinLock lg(data.m_lock);
        if (data.m_current == text){
            return std::string();
        }
        error = data.process(text, value);
        data.m_current = std::move(text);
        data.m_value = value;
        data.m_error.clear();
    }
    report_value_changed(this);
    return error;
}
template <typename Type>
std::string TimeExpressionCell<Type>::time_string() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    if (!data.m_error.empty()){
        return "<font color=\"red\">" + data.m_error + "</font>";
    }
    return ticks_to_time(data.m_ticks_per_second, data.m_value);
}

template <typename Type>
void TimeExpressionCell<Type>::load_json(const JsonValue& json){
    Data& data = *m_data;
    const std::string* str = json.to_string();
    if (str == nullptr){
        return;
    }
    {
        WriteSpinLock lg(data.m_lock);
        data.m_current = *str;
        data.m_error = data.process(data.m_current, data.m_value);
    }
    report_value_changed(this);
}
template <typename Type>
JsonValue TimeExpressionCell<Type>::to_json() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_current;
}

template <typename Type>
std::string TimeExpressionCell<Type>::check_validity() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_error;
}
template <typename Type>
void TimeExpressionCell<Type>::restore_defaults(){
    Data& data = *m_data;
    {
        WriteSpinLock lg(data.m_lock);
        data.m_current = data.m_default;
        data.m_error = data.process(data.m_current, data.m_value);
    }
    report_value_changed(this);
}




template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    std::string label,
    LockMode lock_while_running,
    double ticks_per_second,
    Type min_value, Type max_value,
    std::string default_value
)
    : TimeExpressionCell<Type>(
        lock_while_running,
        ticks_per_second,
        min_value, max_value,
        std::move(default_value)
    )
    , m_label(std::move(label))
{}
template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    std::string label,
    LockMode lock_while_running,
    double ticks_per_second,
    std::string default_value
)
    : TimeExpressionCell<Type>(
        lock_while_running,
        ticks_per_second,
        std::move(default_value)
    )
    , m_label(std::move(label))
{}
template <typename Type>
TimeExpressionOption<Type>::TimeExpressionOption(
    std::string label,
    LockMode lock_while_running,
    double ticks_per_second,
    Type min_value,
    std::string default_value
)
    : TimeExpressionCell<Type>(
        lock_while_running,
        ticks_per_second,
        min_value,
        std::move(default_value)
    )
    , m_label(std::move(label))
{}







template <typename Type>
ConfigWidget* TimeExpressionCell<Type>::make_QtWidget(QWidget& parent){
    return new TimeExpressionCellWidget<Type>(parent, *this);
}

template <typename Type>
ConfigWidget* TimeExpressionOption<Type>::make_QtWidget(QWidget& parent){
    return new TimeExpressionOptionWidget<Type>(parent, *this);
}


template class TimeExpressionCell<uint8_t>;
template class TimeExpressionCell<uint16_t>;
template class TimeExpressionCell<uint32_t>;
template class TimeExpressionCell<int16_t>;
template class TimeExpressionCell<int32_t>;

template class TimeExpressionOption<uint8_t>;
template class TimeExpressionOption<uint16_t>;
template class TimeExpressionOption<uint32_t>;
template class TimeExpressionOption<int16_t>;
template class TimeExpressionOption<int32_t>;



}
