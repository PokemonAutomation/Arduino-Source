/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SettingsPanel.h"


namespace PokemonAutomation{


SettingsPanelInstance::SettingsPanelInstance(const PanelDescriptor& descriptor)
    : PanelInstance(descriptor)
    , m_options(LockMode::LOCK_WHILE_RUNNING)
{}

void SettingsPanelInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue SettingsPanelInstance::to_json() const{
    return m_options.to_json();
}







}
