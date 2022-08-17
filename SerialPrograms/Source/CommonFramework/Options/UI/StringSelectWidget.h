/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringSelectWidget_H
#define PokemonAutomation_Options_StringSelectWidget_H

#include <QWidget>
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{


class StringSelectCellWidget : public NoWheelComboBox, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StringSelectCellWidget();
    StringSelectCellWidget(QWidget& parent, StringSelectCell& value);

    virtual void update() override;
    virtual void value_changed() override;

private:
    void load_options();
    void hide_options();

    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    StringSelectCell& m_value;
};



class StringSelectOptionWidget2 : public QWidget, public ConfigWidget{
public:
    StringSelectOptionWidget2(QWidget& parent, StringSelectOption2& value);

    virtual void update() override;

private:
    StringSelectCellWidget* m_cell;
};



}
#endif
