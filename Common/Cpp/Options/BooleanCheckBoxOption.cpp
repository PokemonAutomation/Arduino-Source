/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "BooleanCheckBoxOption.h"

namespace PokemonAutomation{



BooleanCheckBoxCell::BooleanCheckBoxCell(bool default_value)
    : m_default(default_value)
    , m_current(default_value)
{}
void BooleanCheckBoxCell::operator=(bool x){
    m_current.store(x, std::memory_order_relaxed);
    push_update();
}
void BooleanCheckBoxCell::load_json(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        *this = value;
        push_update();
    }
}
JsonValue BooleanCheckBoxCell::to_json() const{
    return (bool)*this;
}
void BooleanCheckBoxCell::restore_defaults(){
    *this = m_default;
    push_update();
}




BooleanCheckBoxOption::BooleanCheckBoxOption(std::string label, bool default_value)
    : BooleanCheckBoxCell(default_value)
    , m_label(std::move(label))
{}



}
