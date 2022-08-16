/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigWidget_H
#define PokemonAutomation_ConfigWidget_H

#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class ConfigWidget{
public:
    virtual ~ConfigWidget() = default;
    ConfigWidget(ConfigOption& m_value);
    ConfigWidget(ConfigOption& m_value, QWidget& widget);

    const ConfigOption& option() const{ return m_value; }
    ConfigOption& option(){ return m_value; }

    QWidget& widget(){ return *m_widget; }

    //  Needs to be called on the UI thread.
    virtual void update();

protected:
    ConfigOption& m_value;
    QWidget* m_widget = nullptr;
};



}
#endif
