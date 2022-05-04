/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "SettingsPanel.h"
#include "SettingsPanelWidget.h"


namespace PokemonAutomation{



void SettingsPanelInstance::from_json(const QJsonValue& json){
    m_options.load_json(json);
}
QJsonValue SettingsPanelInstance::to_json() const{
    return m_options.to_json();
}
QWidget* SettingsPanelInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return SettingsPanelWidget::make(parent, *this, holder);
}







}
