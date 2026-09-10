/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/ResourceDownload/ResourceDownloadHelpers.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Logging/Logger.h"
#include "PanelSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


PanelSession::PanelSession(const PanelDescriptor& descriptor)
    : m_descriptor(descriptor)
{
    try{
        validate_resource_list();
    }catch (FileException& e){
        e.log(global_logger_tagged());
    }
}

void PanelSession::from_json(){
    JsonValue* node = PERSISTENT_SETTINGS().panels.get_value(m_descriptor.identifier());
    if (node == nullptr){
        return;
    }
    from_json(*node);
}

void PanelSession::from_json(const JsonValue& json){

}
JsonValue PanelSession::to_json() const{
    return JsonValue();
}
void PanelSession::save_settings() const{
    const std::string& identifier = m_descriptor.identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[identifier] = to_json();
    }
    global_logger_tagged().log("Saving panel settings...");
    PERSISTENT_SETTINGS().write();
}


void PanelSession::validate_resource_list(){
    const std::unordered_set<std::string>& master_list = all_resource_names();

    for (const std::string& resource_string : m_descriptor.required_resources()){
        if (!master_list.contains(resource_string)){
            throw InternalProgramError(
                nullptr,
                PA_CURRENT_FUNCTION,
                "validate_resource_list: Invalid resource in descriptor."
            );
        }
    }
}






}
