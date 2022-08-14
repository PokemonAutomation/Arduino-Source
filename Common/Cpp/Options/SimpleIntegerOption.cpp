/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SimpleIntegerOption.h"

#include "Common/Qt/Options/SimpleIntegerWidget.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    std::string label,
    Type default_value,
    Type min_value,
    Type max_value
)
    : m_label(std::move(label))
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_default(default_value)
{}

template <typename Type>
std::string SimpleIntegerOption<Type>::set(Type x){
    std::string err = check_validity(x);
    if (err.empty()){
        m_current.store(x, std::memory_order_relaxed);
        push_update();
    }
    return err;
}
template <typename Type>
void SimpleIntegerOption<Type>::load_json(const JsonValue& json){
    Type value;
    if (json.read_integer(value, m_min_value, m_max_value)){
        set(value);
        m_current.store(value, std::memory_order_relaxed);
        push_update();
    }
}
template <typename Type>
JsonValue SimpleIntegerOption<Type>::to_json() const{
    return (Type)*this;
}

template <typename Type>
std::string SimpleIntegerOption<Type>::check_validity(Type x) const{
    if (x < m_min_value){
        return "Value too small: min = " + std::to_string(m_min_value) + ", value = " + std::to_string(x);
    }
    if (x > m_max_value){
        return "Value too large: max = " + std::to_string(m_max_value) + ", value = " + std::to_string(x);
    }
    return std::string();
}
template <typename Type>
std::string SimpleIntegerOption<Type>::check_validity() const{
    return check_validity(*this);
}
template <typename Type>
void SimpleIntegerOption<Type>::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    push_update();
}




template <typename Type>
ConfigWidget* SimpleIntegerOption<Type>::make_ui(QWidget& parent){
    return new SimpleIntegerWidget<Type>(parent, *this);
}





template class SimpleIntegerOption<uint8_t>;
template class SimpleIntegerOption<uint16_t>;
template class SimpleIntegerOption<uint32_t>;
template class SimpleIntegerOption<uint64_t>;
template class SimpleIntegerOption<int8_t>;



}
