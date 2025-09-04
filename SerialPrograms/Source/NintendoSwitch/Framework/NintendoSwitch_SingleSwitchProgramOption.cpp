/*  Single Switch Program Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_SingleSwitchProgramOption.h"
#include "UI/NintendoSwitch_SingleSwitchProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SingleSwitchProgramOption::~SingleSwitchProgramOption() = default;
SingleSwitchProgramOption::SingleSwitchProgramOption(const SingleSwitchProgramDescriptor& descriptor)
    : PanelInstance(descriptor)
    , m_descriptor(descriptor)
    , m_system(descriptor.allow_commands_while_running())
    , m_instance(descriptor.make_instance())
{}

void SingleSwitchProgramOption::from_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("SwitchSetup");
    if (value){
        m_system.load_json(*value);
    }
    m_instance->from_json(json);
}
JsonValue SingleSwitchProgramOption::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["SwitchSetup"] = m_system.to_json();
    return obj;
}

ConfigOption& SingleSwitchProgramOption::options(){
    return m_instance->m_options;
}

std::string SingleSwitchProgramOption::check_validity() const{
    return m_instance->check_validity();
}
void SingleSwitchProgramOption::restore_defaults(){
    m_instance->restore_defaults();
}


QWidget* SingleSwitchProgramOption::make_widget(QWidget& parent, PanelHolder& holder){
    return new SingleSwitchProgramWidget2(parent, *this, holder);
}





}
}
