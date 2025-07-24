/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConfigWidget_H
#define PokemonAutomation_ConfigWidget_H

#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


//  Base class for all config widgets that control a program option.
//
//  It is intuitive to let ConfigWidget inherit QWidget, base class of Qt UI element,
//  But some derived classes of ConfigWidget, e.g. StringCellWidget, needs to inherit
//  QWidget's derived classes, e.g. QLineEdit. To avoid nasty diamond inheritance,
//  ConfigWidget does not inherit QWidget. User has to call ConfigWidget::widget()
//  to get the actual QWidget.
//
//  ConfigWidget's derived classes need to inherit a QWidget or its derived class
//  and pass *this as the widget in ConfigWidget(m_valuie, widget) so a ConfigWidget
//  pointer can get the actual QWidget.
class ConfigWidget : protected ConfigOption::Listener{
public:
    virtual ~ConfigWidget();
    ConfigWidget(ConfigOption& m_value);
    ConfigWidget(ConfigOption& m_value, QWidget& widget);

    const ConfigOption& option() const{ return m_value; }
    ConfigOption& option(){ return m_value; }

    QWidget& widget(){ return *m_widget; }

    //  Needs to be called on the UI thread.
    virtual void update_value(){}
    virtual void update_visibility();
    void update_visibility(bool program_is_running);
    void update_all(bool program_is_running);

protected:
    virtual void on_config_visibility_changed() override;
    virtual void on_program_state_changed(bool program_is_running) override;

protected:
    ConfigOption& m_value;
    QWidget* m_widget = nullptr;
    bool m_program_is_running = false;
};



}
#endif
