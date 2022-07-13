/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SimpleIntegerBaseOption.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerBaseOption<Type>::SimpleIntegerBaseOption(
    std::string label,
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
std::string SimpleIntegerBaseOption<Type>::set(Type x){
    std::string err = check_validity(x);
    if (err.empty()){
        m_current.store(x, std::memory_order_relaxed);
    }
    return err;
}

template <typename Type>
std::string SimpleIntegerBaseOption<Type>::check_validity(Type x) const{
    if (x < m_min_value){
        return "Value too small: min = " + std::to_string(m_min_value) + ", value = " + std::to_string(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + std::to_string(m_max_value) + ", value = " + std::to_string(x);
    }
    return std::string();
}
template <typename Type>
void SimpleIntegerBaseOption<Type>::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}


template <typename Type>
void SimpleIntegerBaseOption<Type>::load_default(const JsonValue& json){
    json.read_integer(m_default, m_min_value, m_max_value);
}
template <typename Type>
void SimpleIntegerBaseOption<Type>::load_current(const JsonValue& json){
    Type value;
    if (json.read_integer(value, m_min_value, m_max_value)){
        m_current.store(value, std::memory_order_relaxed);
    }
}
template <typename Type>
JsonValue SimpleIntegerBaseOption<Type>::write_default() const{
    return m_default;
}
template <typename Type>
JsonValue SimpleIntegerBaseOption<Type>::write_current() const{
    return m_current.load(std::memory_order_relaxed);
}

template <typename Type>
std::string SimpleIntegerBaseOption<Type>::check_validity() const{
    Type current = m_current.load(std::memory_order_relaxed);
    return check_validity(current);
}





template class SimpleIntegerBaseOption<uint8_t>;
template class SimpleIntegerBaseOption<uint16_t>;
template class SimpleIntegerBaseOption<uint32_t>;
template class SimpleIntegerBaseOption<uint64_t>;
template class SimpleIntegerBaseOption<int8_t>;



}
