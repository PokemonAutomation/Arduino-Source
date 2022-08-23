/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "GroupOption.h"

namespace PokemonAutomation{


GroupOption::GroupOption(
    std::string label,
    bool toggleable,
    bool enabled
)
    : m_label(std::move(label))
    , m_toggleable(toggleable)
    , m_default_enabled(enabled)
    , m_enabled(enabled)
{}
bool GroupOption::enabled() const{
    return m_enabled.load(std::memory_order_relaxed);
}
void GroupOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    if (m_toggleable){
        bool enabled;
        if (obj->read_boolean(enabled, "Enabled")){
            m_enabled.store(enabled, std::memory_order_relaxed);
            on_set_enabled(enabled);
        }
    }
}
JsonValue GroupOption::to_json() const{
    JsonObject obj = std::move(*BatchOption::to_json().get_object());
    if (m_toggleable){
        obj["Enabled"] = m_enabled.load(std::memory_order_relaxed);
    }
    return obj;
}
void GroupOption::restore_defaults(){
    m_enabled.store(m_default_enabled, std::memory_order_relaxed);
    BatchOption::restore_defaults();
}
void GroupOption::on_set_enabled(bool enabled){}





}
