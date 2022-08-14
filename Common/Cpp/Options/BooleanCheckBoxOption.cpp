/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "BooleanCheckBoxOption.h"

namespace PokemonAutomation{



BooleanCheckBoxOption::BooleanCheckBoxOption(
    std::string label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}


void BooleanCheckBoxOption::operator=(bool x){
    m_current.store(x, std::memory_order_relaxed);
    push_update();
}
void BooleanCheckBoxOption::load_json(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        *this = value;
        push_update();
    }
}
JsonValue BooleanCheckBoxOption::to_json() const{
    return (bool)*this;
}

void BooleanCheckBoxOption::restore_defaults(){
    *this = m_default;
    push_update();
}




}
