/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Logging/Logger.h"
#include "PanelInstance.h"

namespace PokemonAutomation{


PanelInstance::PanelInstance(const PanelDescriptor& descriptor)
    : m_descriptor(descriptor)
{}

void PanelInstance::from_json(){
    JsonValue* node = PERSISTENT_SETTINGS().panels.get_value(m_descriptor.identifier());
    if (node == nullptr){
        return;
    }
    from_json(*node);
}
JsonValue PanelInstance::to_json() const{
    return JsonValue();
}
void PanelInstance::save_settings() const{
    const std::string& identifier = m_descriptor.identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[identifier] = to_json();
    }
    global_logger_tagged().log("Saving panel settings...");
    PERSISTENT_SETTINGS().write();
}



}
