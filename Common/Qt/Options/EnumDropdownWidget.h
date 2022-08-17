/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownWidget_H
#define PokemonAutomation_Options_EnumDropdownWidget_H

#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{


class EnumDropdownCellWidget : public NoWheelComboBox, public ConfigWidget, private ConfigOption::Listener{
public:
    ~EnumDropdownCellWidget();
    EnumDropdownCellWidget(QWidget& parent, DropdownCell& value);

    virtual void update() override;
    virtual void value_changed() override;

protected:
    DropdownCell& m_value;
};


class EnumDropdownOptionWidget : public QWidget, public ConfigWidget{
public:
    EnumDropdownOptionWidget(QWidget& parent, DropdownOption& value);

    virtual void update() override;

private:
    EnumDropdownCellWidget* m_cell;
};





}
#endif
