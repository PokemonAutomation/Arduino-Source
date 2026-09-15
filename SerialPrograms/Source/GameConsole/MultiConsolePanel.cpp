/*  Multi-Console Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Framework/ConsoleSystemSession.h"
#include "Framework/MultiConsolePanelSession.h"
#include "MultiConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{


MultiConsolePanelDescriptor::MultiConsolePanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelDeprecation deprecation,
    size_t min_consoles,
    size_t max_consoles,
    size_t default_consoles,
    bool restore_defaults_button
)
    : MultiConsolePanelDescriptor(
        color,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation,
        min_consoles,
        max_consoles,
        default_consoles,
        restore_defaults_button,
        [](size_t console_index){
            return std::make_unique<ConsoleSystemOption>(1);
        },
        [](ConsoleSystemOption& option, size_t console_index){
            return std::make_unique<ConsoleSystemSession>(
                global_logger_raw(),
                option,
                true,
                console_index,
                std::nullopt
            );
        }
    )
{}
MultiConsolePanelDescriptor::MultiConsolePanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelDeprecation deprecation,
    size_t min_consoles,
    size_t max_consoles,
    size_t default_consoles,
    bool restore_defaults_button,
    OptionFactory option_factory,
    SessionFactory session_factory
)
    : PanelDescriptor(
        color,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation,
        restore_defaults_button,
        {}
    )
    , m_min_consoles(min_consoles)
    , m_max_consoles(max_consoles)
    , m_default_consoles(default_consoles)
    , m_option_factory(std::move(option_factory))
    , m_session_factory(std::move(session_factory))
{}
std::unique_ptr<PanelSession> MultiConsolePanelDescriptor::make_panel() const{
    return std::make_unique<MultiConsolePanelSession>(*this);
}



MultiConsolePanelInstance::MultiConsolePanelInstance()
    : m_options(LockMode::UNLOCK_WHILE_RUNNING)
{}

void MultiConsolePanelInstance::add_option(ConfigOption& option, std::string serialization_string){
    m_options.add_option(option, std::move(serialization_string));
}

std::string MultiConsolePanelInstance::check_validity() const{
    return m_options.check_validity();
}
void MultiConsolePanelInstance::restore_defaults(){
    return m_options.restore_defaults();
}
JsonValue MultiConsolePanelInstance::to_json() const{
    return m_options.to_json();
}
void MultiConsolePanelInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}



}
}
