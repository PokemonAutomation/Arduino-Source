/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ResourceDownload/ResourceDownloadHelpers.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Logging/Logger.h"
#include "PanelInstance.h"

#include <iostream>
using std::cout;
using std::endl;
namespace PokemonAutomation{


PanelInstance::PanelInstance(const PanelDescriptor& descriptor)
    : m_descriptor(descriptor)
{
    validate_resource_list();
}

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


void PanelInstance::validate_resource_list(){

    const std::unordered_set<std::string>& master_list = all_resource_names();

    for (std::string resource_string : m_descriptor.required_resources()){
        if (!master_list.contains(resource_string)){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "validate_resource_list: Invalid resource in descriptor."); 
        }
    }
}


}
