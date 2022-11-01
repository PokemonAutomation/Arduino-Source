/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigWidget_H
#define PokemonAutomation_ConfigWidget_H

#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


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
    virtual void update_visibility(bool program_is_running);
    void update_all(bool program_is_running);

protected:
    virtual void visibility_changed() override;
    virtual void program_state_changed(bool program_is_running) override;

protected:
    ConfigOption& m_value;
    QWidget* m_widget = nullptr;
    bool m_program_is_running = false;
};



}
#endif
