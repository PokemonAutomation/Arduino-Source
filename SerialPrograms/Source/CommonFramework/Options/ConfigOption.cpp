/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include <QWidget>
#include "ConfigOption.h"

namespace PokemonAutomation{




QString ConfigOption::check_validity() const{
    return QString();
}


ConfigWidget::ConfigWidget(ConfigOption& m_value, QWidget& widget)
    : m_value(m_value)
    , m_widget(widget)
{
    ConfigWidget::update_visibility();
}
void ConfigWidget::update_visibility(){
    switch (m_value.visibility){
    case ConfigOptionState::ENABLED:
        m_widget.setEnabled(true);
        m_widget.setVisible(true);
        break;
    case ConfigOptionState::DISABLED:
        m_widget.setEnabled(false);
        m_widget.setVisible(true);
        break;
    case ConfigOptionState::HIDDEN:
        m_widget.setEnabled(false);
        m_widget.setVisible(false);
        break;
    }
}






}
