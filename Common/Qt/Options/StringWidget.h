/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringWidget_H
#define PokemonAutomation_Options_StringWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/StringOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class StringCellWidget : public QLineEdit, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StringCellWidget();
    StringCellWidget(QWidget& parent, StringCell& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    StringCell& m_value;
};



class StringOptionWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StringOptionWidget();
    StringOptionWidget(QWidget& parent, StringOption& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    StringOption& m_value;
    QLineEdit* m_box;
};





}
#endif
