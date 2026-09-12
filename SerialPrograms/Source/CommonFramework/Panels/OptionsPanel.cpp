/*  Options Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "OptionsPanel.h"
#include "OptionsPanelSession.h"

namespace PokemonAutomation{



OptionsPanelDescriptor::OptionsPanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelDeprecation deprecation,
    bool restore_defaults_button
)
    : PanelDescriptor(
        color,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation
    )
    , m_restore_defaults_button(restore_defaults_button)
{}
std::unique_ptr<PanelSession> OptionsPanelDescriptor::make_panel() const{
    return std::make_unique<OptionsPanelSession>(*this);
}




OptionsPanelInstance::OptionsPanelInstance()
    : m_options(LockMode::LOCK_WHILE_RUNNING)
{}

void OptionsPanelInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
void OptionsPanelInstance::restore_defaults(){
    m_options.restore_defaults();
}
JsonValue OptionsPanelInstance::to_json() const{
    return m_options.to_json();
}
void OptionsPanelInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}







}
