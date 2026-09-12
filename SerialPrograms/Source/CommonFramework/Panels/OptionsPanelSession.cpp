/*  Options Panel Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "OptionsPanelSession.h"

namespace PokemonAutomation{



OptionsPanelSession::OptionsPanelSession(const OptionsPanelDescriptor& descriptor)
    : UiState<OptionsPanelSession, PanelSession>(descriptor)
    , m_descriptor(descriptor)
    , m_instance(descriptor.make_instance())
{}

ConfigOption& OptionsPanelSession::options(){
    return m_instance->m_options;
}
void OptionsPanelSession::restore_defaults(){
    m_instance->restore_defaults();
}


JsonValue OptionsPanelSession::to_json() const{
    return m_instance->to_json();
}
void OptionsPanelSession::load_json(const JsonValue& json){
    m_instance->load_json(json);
}



}
