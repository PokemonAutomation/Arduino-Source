/*  Console Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ConsolePanelSession.h"

namespace PokemonAutomation{
namespace GameConsole{



bool ConsolePanelSession::try_shutdown(){
    return m_system.try_shutdown();
}
ConsolePanelSession::~ConsolePanelSession(){
    blocking_shutdown(
        m_system.logger(),
        "ConsolePanelSession",
        [this]{ return try_shutdown(); }
    );
}
ConsolePanelSession::ConsolePanelSession(const ConsolePanelDescriptor& descriptor)
    : UiState<ConsolePanelSession, PanelSession>(descriptor)
    , m_descriptor(descriptor)
    , m_system_option(descriptor.num_controllers(), true)
    , m_system(global_logger_raw(), m_system_option, 0)
    , m_instance(descriptor.make_instance(m_system))
{}

ConfigOption& ConsolePanelSession::options(){
    return m_instance->m_options;
}
void ConsolePanelSession::restore_defaults(){
    m_instance->restore_defaults();
}


JsonValue ConsolePanelSession::to_json() const{
    JsonObject obj = std::move(*m_instance->to_json().to_object());
    obj["ConsoleSetup"] = m_system_option.to_json();
    return obj;
}
void ConsolePanelSession::load_json(const JsonValue& json){
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
