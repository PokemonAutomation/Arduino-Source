/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SettingsPanel.h"


namespace PokemonAutomation{


SettingsPanelInstance::SettingsPanelInstance(const PanelDescriptor& descriptor)
    : PanelSession(descriptor)
    , m_options(LockMode::LOCK_WHILE_RUNNING)
{}

JsonValue SettingsPanelInstance::to_json() const{
    return m_options.to_json();
}
void SettingsPanelInstance::load_json(const JsonValue& json){
    m_options.load_json(json);
}







}
