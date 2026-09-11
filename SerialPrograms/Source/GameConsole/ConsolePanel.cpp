/*  Console Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "ConsolePanel.h"
#include "ConsolePanelSession.h"

namespace PokemonAutomation{
namespace GameConsole{



ConsolePanelDescriptor::ConsolePanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    size_t num_controllers,
    bool deprecated,
    std::vector<std::string> required_resources
)
    : PanelDescriptor(
        color,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        std::move(required_resources)
    )
    , m_deprecated(deprecated)
    , m_num_controllers(num_controllers)
{}
std::unique_ptr<PanelSession> ConsolePanelDescriptor::make_panel() const{
    return std::make_unique<ConsolePanelSession>(*this);
}



ConsolePanelInstance::ConsolePanelInstance()
    : m_options(LockMode::UNLOCK_WHILE_RUNNING)
{}

void ConsolePanelInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}
JsonValue ConsolePanelInstance::to_json() const{
    return m_options.to_json();
}
void ConsolePanelInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}
std::string ConsolePanelInstance::check_validity() const{
    return m_options.check_validity();
}
void ConsolePanelInstance::restore_defaults(){
    return m_options.restore_defaults();
}








}
}
