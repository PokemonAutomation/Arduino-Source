/*  Boolean Check Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "BooleanCheckBoxBaseOption.h"

namespace PokemonAutomation{


BooleanCheckBoxBaseOption::BooleanCheckBoxBaseOption(
    QString label,
    bool default_value
)
    : m_label(std::move(label))
    , m_default(default_value)
    , m_current(default_value)
{}


void BooleanCheckBoxBaseOption::load_default(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        m_default = value;
    }
}
void BooleanCheckBoxBaseOption::load_current(const JsonValue& json){
    bool value;
    if (json.read_boolean(value)){
        m_current.store(value, std::memory_order_relaxed);
    }
}
JsonValue BooleanCheckBoxBaseOption::write_default() const{
    return m_default;
}
JsonValue BooleanCheckBoxBaseOption::write_current() const{
    return m_current.load(std::memory_order_relaxed);
}

void BooleanCheckBoxBaseOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
}





}

