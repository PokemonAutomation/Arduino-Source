/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "SimpleIntegerOption.h"

#include "Common/Qt/Options/SimpleIntegerWidget.h"

namespace PokemonAutomation{




template <typename Type>
ConfigWidget* SimpleIntegerCell<Type>::make_QtWidget(QWidget& parent){
    return new SimpleIntegerCellWidget<Type>(parent, *this);
}
template <typename Type>
ConfigWidget* SimpleIntegerOption<Type>::make_QtWidget(QWidget& parent){
    return new SimpleIntegerOptionWidget<Type>(parent, *this);
}



template <typename Type>
struct SimpleIntegerCell<Type>::Data{
    const Type m_min_value;
    const Type m_max_value;
    const Type m_default;
    std::atomic<Type> m_current;

    Data(Type min_value, Type max_value, Type default_value, Type value)
        : m_min_value(min_value)
        , m_max_value(max_value)
        , m_default(default_value)
        , m_current(value)
    {}
};




template <typename Type>
SimpleIntegerCell<Type>::~SimpleIntegerCell() = default;
template <typename Type>
SimpleIntegerCell<Type>::SimpleIntegerCell(const SimpleIntegerCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.min_value(), x.max_value(), x.default_value(), x.current_value())
{}
template <typename Type>
SimpleIntegerCell<Type>::SimpleIntegerCell(
    LockMode lock_while_running,
    Type min_value, Type max_value,
    Type default_value, Type current_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, current_value)
{}

template <typename Type>
SimpleIntegerCell<Type>::SimpleIntegerCell(
    LockMode lock_while_running,
    Type default_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, std::numeric_limits<Type>::min(), std::numeric_limits<Type>::max(), default_value, default_value)
{}
template <typename Type>
SimpleIntegerCell<Type>::SimpleIntegerCell(
    LockMode lock_while_running,
    Type default_value, Type min_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, std::numeric_limits<Type>::max(), default_value, default_value)
{}
template <typename Type>
SimpleIntegerCell<Type>::SimpleIntegerCell(
    LockMode lock_while_running,
    Type default_value, Type min_value, Type max_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, min_value, max_value, default_value, default_value)
{}

template <typename Type>
Type SimpleIntegerCell<Type>::min_value() const{
    return m_data->m_min_value;
}
template <typename Type>
Type SimpleIntegerCell<Type>::max_value() const{
    return m_data->m_max_value;
}
template <typename Type>
Type SimpleIntegerCell<Type>::default_value() const{
    return m_data->m_default;
}
template <typename Type>
Type SimpleIntegerCell<Type>::current_value() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
template <typename Type>
SimpleIntegerCell<Type>::operator Type() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
template <typename Type>
std::string SimpleIntegerCell<Type>::set(Type x){
    std::string err = check_validity(x);
    if (!err.empty()){
        return err;
    }
    if ((Type)*this == x){
        return std::string();
    }
    if (x != m_data->m_current.exchange(x, std::memory_order_relaxed)){
        report_value_changed(this);
    }
    return std::string();
}
template <typename Type>
void SimpleIntegerCell<Type>::load_json(const JsonValue& json){
    Type value;
    if (json.read_integer(value, m_data->m_min_value, m_data->m_max_value)){
        set(value);
    }
}
template <typename Type>
JsonValue SimpleIntegerCell<Type>::to_json() const{
    return (Type)*this;
}

template <typename Type>
std::string SimpleIntegerCell<Type>::check_validity(Type x) const{
    if (x < m_data->m_min_value){
        return "Value too small: min = " + std::to_string(m_data->m_min_value) + ", value = " + std::to_string(x);
    }
    if (x > m_data->m_max_value){
        return "Value too large: max = " + std::to_string(m_data->m_max_value) + ", value = " + std::to_string(x);
    }
    return std::string();
}
template <typename Type>
std::string SimpleIntegerCell<Type>::check_validity() const{
    return check_validity(*this);
}
template <typename Type>
void SimpleIntegerCell<Type>::restore_defaults(){
    set(m_data->m_default);
}





template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    LockMode lock_while_running,
    Type min_value, Type max_value,
    Type default_value, Type current_value
)
    : SimpleIntegerCell<Type>(lock_while_running, min_value, max_value, default_value, current_value)
    , m_label(std::move(label))
{}
template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    LockMode lock_while_running,
    Type default_value
)
    : SimpleIntegerCell<Type>(lock_while_running, default_value)
    , m_label(std::move(label))
{}
template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    LockMode lock_while_running,
    Type default_value, Type min_value
)
    : SimpleIntegerCell<Type>(lock_while_running, default_value, min_value)
    , m_label(std::move(label))
{}
template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    LockMode lock_while_running,
    Type default_value, Type min_value, Type max_value
)
    : SimpleIntegerCell<Type>(lock_while_running, default_value, min_value, max_value)
    , m_label(std::move(label))
{}






template class SimpleIntegerCell<uint8_t>;
template class SimpleIntegerCell<uint16_t>;
template class SimpleIntegerCell<uint32_t>;
template class SimpleIntegerCell<uint64_t>;
template class SimpleIntegerCell<int8_t>;
template class SimpleIntegerCell<int16_t>;
template class SimpleIntegerCell<int32_t>;
template class SimpleIntegerCell<int64_t>;

template class SimpleIntegerOption<uint8_t>;
template class SimpleIntegerOption<uint16_t>;
template class SimpleIntegerOption<uint32_t>;
template class SimpleIntegerOption<uint64_t>;
template class SimpleIntegerOption<int8_t>;
template class SimpleIntegerOption<int16_t>;
template class SimpleIntegerOption<int32_t>;
template class SimpleIntegerOption<int64_t>;


//  This is stupid.
#ifdef __APPLE__
template class SimpleIntegerCell<size_t>;
template class SimpleIntegerOption<size_t>;
#endif


}
