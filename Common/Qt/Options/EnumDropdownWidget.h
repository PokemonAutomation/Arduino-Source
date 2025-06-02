/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownWidget_H
#define PokemonAutomation_Options_EnumDropdownWidget_H

#include <map>
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{


class EnumDropdownCellWidget : public NoWheelComboBox, public ConfigWidget{
public:
    ~EnumDropdownCellWidget();
    EnumDropdownCellWidget(QWidget& parent, IntegerEnumDropdownCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

protected:
    IntegerEnumDropdownCell& m_value;
    std::map<size_t, int> m_value_to_index;
    std::vector<size_t> m_index_to_value;
};


class EnumDropdownOptionWidget : public QWidget, public ConfigWidget{
public:
    EnumDropdownOptionWidget(QWidget& parent, IntegerEnumDropdownOption& value);

    virtual void update_value() override;

private:
    EnumDropdownCellWidget* m_cell;
};





}
#endif
