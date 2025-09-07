/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ExpressionEvaluator.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Qt/Options/TimeDurationWidget.h"
#include "TimeDurationOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


template <typename Type>
struct TimeDurationCell<Type>::Data{
    const std::string m_units;
    const bool m_show_summary;
    const Type m_min_value;
    const Type m_max_value;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
    Type m_value;
    std::string m_error;

    Data(
        std::string units, bool show_summary,
        Type min_value, Type max_value,
        std::string default_value
    )
        : m_units(units)
        , m_show_summary(show_summary)
        , m_min_value(min_value)
        , m_max_value(max_value)
        , m_default(std::move(default_value))
        , m_current(m_default)
        , m_value(0)
    {
        m_error = process(m_current, m_value);
    }

    static std::map<std::string, int64_t> make_symbol_map(){
        std::map<std::string, int64_t> map;

        auto self = std::chrono::duration_cast<std::chrono::nanoseconds>(Type(1)).count();
        auto microseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::microseconds(1)).count();
        auto milliseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();
        auto seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();
        auto minutes = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::minutes(1)).count();
        auto hours = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::hours(1)).count();
        auto days = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::days(1)).count();
        auto weeks = days * 7;
        auto years = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::years(1)).count();

        if (self <= microseconds){
            map["us"] = map["microsecond"] = map["microseconds"] = microseconds / self;
        }
        if (self <= milliseconds){
            map["ms"] = map["millisecond"] = map["milliseconds"] = milliseconds / self;
        }
        if (self <= seconds){
            map["s"] = map["secs"] = map["second"] = map["seconds"] = seconds / self;
        }
        if (self <= minutes){
            map["min"] = map["minute"] = map["minutes"] = minutes / self;
        }
        if (self <= hours){
            map["h"] = map["hour"] = map["hours"] = hours / self;
        }
        if (self <= days){
            map["d"] = map["day"] = map["days"] = days / self;
        }
        if (self <= weeks){
            map["w"] = map["week"] = map["weeks"] = weeks / self;
        }
        if (self <= years){
            map["y"] = map["year"] = map["years"] = years / self;
        }

        return map;
    }

    std::string process(const std::string& text, Type& value) const{
        if (text.empty()){
            return "Expression is empty.";
        }

        static const std::map<std::string, int64_t> SYMBOLS = make_symbol_map();

        using Rep = typename Type::rep;
        Rep parsed;
        try{
            parsed = parse_expression(SYMBOLS, text);
        }catch (const ParseException& str){
            return str.message();
        }
        // std::cout << "value = " << parsed << " " << m_min_value << " " << m_max_value << std::endl;

        if (Type(parsed) < m_min_value){
            return "Overflow: Number is too small.";
        }
        if (Type(parsed) > m_max_value){
            return "Overflow: Number is too large.";
        }
        value = (Type)parsed;
        return std::string();
    }
};





template <typename Type>
TimeDurationCell<Type>::~TimeDurationCell() = default;
template <typename Type>
TimeDurationCell<Type>::TimeDurationCell(
    std::string units, bool show_summary,
    LockMode lock_while_running,
    Type min_value, Type max_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::move(units), show_summary,
        min_value, max_value,
        std::move(default_value)
    )
{}

template <typename Type>
TimeDurationCell<Type>::TimeDurationCell(
    std::string units,
    LockMode lock_while_running,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::move(units), true,
        Type(0), Type::max(),
        std::move(default_value)
    )
{}
template <typename Type>
TimeDurationCell<Type>::TimeDurationCell(
    std::string units,
    LockMode lock_while_running,
    Type min_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::move(units), true,
        min_value, Type::max(),
        std::move(default_value)
    )
{}

template <typename Type>
TimeDurationCell<Type>::TimeDurationCell(
    std::string units,
    LockMode lock_while_running,
    Type min_value, Type max_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::move(units), true,
        min_value, max_value,
        std::move(default_value)
    )
{}


template <typename Type>
const std::string& TimeDurationCell<Type>::units() const{
    return m_data->m_units;
}
template <typename Type>
bool TimeDurationCell<Type>::show_summary() const{
    return m_data->m_show_summary;
}
template <typename Type>
Type TimeDurationCell<Type>::min_value() const{
    return m_data->m_min_value;
}
template <typename Type>
Type TimeDurationCell<Type>::max_value() const{
    return m_data->m_max_value;
}
template <typename Type>
const std::string& TimeDurationCell<Type>::default_value() const{
    return m_data->m_default;
}
template <typename Type>
std::string TimeDurationCell<Type>::current_text() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_current;
}
template <typename Type>
Type TimeDurationCell<Type>::get() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_value;
}
template <typename Type>
std::string TimeDurationCell<Type>::set(std::string text){
    Data& data = *m_data;
    Type value(0);
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
std::string TimeDurationCell<Type>::time_string() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    if (!data.m_error.empty()){
        return "<font color=\"red\">" + data.m_error + "</font>";
    }
    return time_string(data.m_current);
}
template <typename Type>
std::string TimeDurationCell<Type>::time_string(const std::string& text) const{
    const Data& data = *m_data;
    Type value{};
    std::string error = data.process(text, value);
    if (!error.empty()){
        return "<font color=\"red\">" + error + "</font>";
    }

    constexpr auto self = std::chrono::duration_cast<std::chrono::nanoseconds>(Type(1)).count();
    constexpr auto milliseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count();
    if constexpr (self >= milliseconds){
        return duration_to_string(std::chrono::duration_cast<Milliseconds>(value));
    }else{
        return tostr_u_commas(value.count()) + " " + m_data->m_units;
    }
}

template <typename Type>
void TimeDurationCell<Type>::load_json(const JsonValue& json){
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
JsonValue TimeDurationCell<Type>::to_json() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_current;
}

template <typename Type>
std::string TimeDurationCell<Type>::check_validity() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_error;
}
template <typename Type>
void TimeDurationCell<Type>::restore_defaults(){
    Data& data = *m_data;
    {
        WriteSpinLock lg(data.m_lock);
        data.m_current = data.m_default;
        data.m_error = data.process(data.m_current, data.m_value);
    }
    report_value_changed(this);
}









template <typename Type>
ConfigWidget* TimeDurationCell<Type>::make_QtWidget(QWidget& parent){
    return new TimeDurationCellWidget<Type>(parent, *this);
}

template <typename Type>
ConfigWidget* TimeDurationOption<Type>::make_QtWidget(QWidget& parent){
    return new TimeDurationOptionWidget<Type>(parent, *this);
}





template class TimeDurationCell<std::chrono::milliseconds>;
template class TimeDurationCell<std::chrono::microseconds>;
template class TimeDurationOption<std::chrono::milliseconds>;
template class TimeDurationOption<std::chrono::microseconds>;







}
