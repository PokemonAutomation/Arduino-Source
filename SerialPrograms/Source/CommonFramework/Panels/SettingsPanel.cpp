/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SettingsPanel.h"
#include "SettingsPanelWidget.h"


namespace PokemonAutomation{


SettingsPanelInstance::SettingsPanelInstance(const PanelDescriptor& descriptor)
    : PanelInstance(descriptor)
    , m_options(LockWhileRunning::LOCKED)
{}

void SettingsPanelInstance::from_json(const JsonValue& json){
    m_options.load_json(json);
}
JsonValue SettingsPanelInstance::to_json() const{
    return m_options.to_json();
}
QWidget* SettingsPanelInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return SettingsPanelWidget::make(parent, *this, holder);
}







}
