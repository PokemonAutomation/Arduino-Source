/*  Multi-Console Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Logging/Logger.h"
#include "MultiConsolePanelSession.h"

namespace PokemonAutomation{
namespace GameConsole{



bool MultiConsolePanelSession::try_shutdown(){
    return m_system.try_shutdown();
}
MultiConsolePanelSession::~MultiConsolePanelSession(){
    blocking_shutdown(
        global_logger_tagged(),
        "ConsolePanelSession",
        [this]{ return try_shutdown(); }
    );
}
MultiConsolePanelSession::MultiConsolePanelSession(const MultiConsolePanelDescriptor& descriptor)
    : UiState<MultiConsolePanelSession, PanelSession>(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(
        descriptor.min_consoles(),
        descriptor.max_consoles(),
        descriptor.default_consoles(),
        descriptor.option_factory()
    )
    , m_system(m_system_option, true)
    , m_instance(descriptor.make_instance(m_system))
{}

ConfigOption& MultiConsolePanelSession::options(){
    return m_instance->m_options;
}


void MultiConsolePanelSession::restore_defaults(){
    m_instance->restore_defaults();
}
JsonValue MultiConsolePanelSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["ConsoleSetup"] = m_system_option.to_json();
    return obj;
}
void MultiConsolePanelSession::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("ConsoleSetup");
    if (value){
        m_system.load_json(*value);
    }
    m_instance->load_json(json);
}



}
}
