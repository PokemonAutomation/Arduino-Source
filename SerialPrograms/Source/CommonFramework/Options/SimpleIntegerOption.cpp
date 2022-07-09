/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/SimpleInteger/SimpleIntegerBaseWidget.h"
#include "SimpleIntegerOption.h"

namespace PokemonAutomation{


template <typename Type>
SimpleIntegerOption<Type>::SimpleIntegerOption(
    QString label,
    Type default_value,
    Type min_value,
    Type max_value
)
    : SimpleIntegerBaseOption<Type>(std::move(label), min_value, max_value, default_value)
{}

template <typename Type>
void SimpleIntegerOption<Type>::load_json(const JsonValue2& json){
    return this->load_current(json);
}
template <typename Type>
JsonValue2 SimpleIntegerOption<Type>::to_json() const{
    return this->write_current();
}

template <typename Type>
QString SimpleIntegerOption<Type>::check_validity() const{
    return SimpleIntegerBaseOption<Type>::check_validity();
}
template <typename Type>
void SimpleIntegerOption<Type>::restore_defaults(){
    SimpleIntegerBaseOption<Type>::restore_defaults();
}





template <typename Type>
class SimpleIntegerWidget : private SimpleIntegerBaseWidget<Type>, public ConfigWidget{
public:
    SimpleIntegerWidget(QWidget& parent, SimpleIntegerOption<Type>& value)
        : SimpleIntegerBaseWidget<Type>(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        SimpleIntegerBaseWidget<Type>::restore_defaults();
    }
    virtual void update_ui() override{
        SimpleIntegerBaseWidget<Type>::update_ui();
    }
};


template <typename Type>
ConfigWidget* SimpleIntegerOption<Type>::make_ui(QWidget& parent){
    return new SimpleIntegerWidget<Type>(parent, *this);
}




template class SimpleIntegerOption<uint8_t>;
template class SimpleIntegerOption<uint16_t>;
template class SimpleIntegerOption<uint32_t>;
template class SimpleIntegerOption<uint64_t>;
template class SimpleIntegerOption<int8_t>;

template class SimpleIntegerWidget<uint8_t>;
template class SimpleIntegerWidget<uint16_t>;
template class SimpleIntegerWidget<uint32_t>;
template class SimpleIntegerWidget<uint64_t>;
template class SimpleIntegerWidget<int8_t>;


}
