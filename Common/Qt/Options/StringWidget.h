/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringWidget_H
#define PokemonAutomation_Options_StringWidget_H

#include <QWidget>
#include "Common/Cpp/Options/StringOption.h"
#include "ConfigWidget.h"

class QLineEdit;

namespace PokemonAutomation{



class StringWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StringWidget();
    StringWidget(QWidget& parent, StringOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    StringOption& m_value;
    QLineEdit * m_box;
};





}
#endif
