/*  Time Duration Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    const Type m_min_value;
    const Type m_max_value;
    const std::string m_default;

    mutable SpinLock m_lock;
    std::string m_current;
    Type m_value;
    std::string m_error;

    Data(
        std::string units,
        Type min_value, Type max_value,
        std::string default_value
    )
        : m_units(units)
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

        static const std::map<std::string, int64_t> SYMBOLS{
            {"ms", std::chrono::duration_cast<Type>(std::chrono::milliseconds(1)).count()},
            {"s", std::chrono::duration_cast<Type>(std::chrono::seconds(1)).count()},
            {"min", std::chrono::duration_cast<Type>(std::chrono::minutes(1)).count()},
            {"h", std::chrono::duration_cast<Type>(std::chrono::hours(1)).count()},
            {"d", std::chrono::duration_cast<Type>(std::chrono::days(1)).count()},
            {"y", std::chrono::duration_cast<Type>(std::chrono::years(1)).count()},
        };

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
    std::string units,
    LockMode lock_while_running,
    Type min_value, Type max_value,
    std::string default_value
)
    : ConfigOption(lock_while_running)
    , m_data(
        CONSTRUCT_TOKEN,
        std::move(units),
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
        std::move(units),
        Type(0), std::chrono::milliseconds::max(),
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
        std::move(units),
        min_value, std::chrono::milliseconds::max(),
        std::move(default_value)
    )
{}


template <typename Type>
const std::string& TimeDurationCell<Type>::units() const{
    return m_data->m_units;
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
TimeDurationCell<Type>::operator Type() const{
    const Data& data = *m_data;
    ReadSpinLock lg(data.m_lock);
    return data.m_value;
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
    return duration_to_string(std::chrono::duration_cast<Milliseconds>(data.m_value));
}
template <typename Type>
std::string TimeDurationCell<Type>::time_string(const std::string& text) const{
    const Data& data = *m_data;
    Type value;
    std::string error = data.process(text, value);
    if (error.empty()){
        return duration_to_string(std::chrono::duration_cast<Milliseconds>(value));
    }else{
        return "<font color=\"red\">" + error + "</font>";
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
template class TimeDurationOption<std::chrono::milliseconds>;







}
