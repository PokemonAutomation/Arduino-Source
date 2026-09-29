/*  Panel Instance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Logging/Logger.h"
#include "PanelSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


PanelSession::PanelSession(const PanelDescriptor& descriptor)
    : m_descriptor(descriptor)
{}

void PanelSession::load_json_from_global(){
    JsonValue* node = PERSISTENT_SETTINGS().panels.get_value(m_descriptor.identifier());
    if (node == nullptr){
        return;
    }
    load_json(*node);
}

JsonValue PanelSession::to_json() const{
    return JsonValue();
}
void PanelSession::load_json(const JsonValue& json){

}
void PanelSession::save_settings() const{
    const std::string& identifier = m_descriptor.identifier();
    if (!identifier.empty()){
        PERSISTENT_SETTINGS().panels[identifier] = to_json();
    }
    global_logger_tagged().log("Saving panel settings...");
    PERSISTENT_SETTINGS().write();
}








}
