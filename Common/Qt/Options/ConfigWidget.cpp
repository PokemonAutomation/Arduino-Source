/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QWidget>
#include "ConfigWidget.h"

namespace PokemonAutomation{


ConfigWidget::ConfigWidget(ConfigOption& m_value)
    : m_value(m_value)
{}
ConfigWidget::ConfigWidget(ConfigOption& m_value, QWidget& widget)
    : m_value(m_value)
    , m_widget(&widget)
{
    ConfigWidget::update();
}
void ConfigWidget::update(){
    if (m_widget == nullptr){
        return;
    }
    switch (m_value.visibility()){
    case ConfigOptionState::ENABLED:
        m_widget->setEnabled(true);
        m_widget->setVisible(true);
        break;
    case ConfigOptionState::DISABLED:
        m_widget->setEnabled(false);
        m_widget->setVisible(true);
        break;
    case ConfigOptionState::HIDDEN:
        m_widget->setEnabled(false);
        m_widget->setVisible(false);
        break;
    }
}



}
