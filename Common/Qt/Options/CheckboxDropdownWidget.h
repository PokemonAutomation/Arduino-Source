/*  Checkbox Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_CheckboxDropdownWidget_H
#define PokemonAutomation_Options_CheckboxDropdownWidget_H

#include "Common/Qt/CheckboxDropdown.h"
#include "Common/Cpp/Options/CheckboxDropdownOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{


class CheckboxDropdownCellWidget : public CheckboxDropdown, public ConfigWidget{
public:
    using ParentOption = CheckboxDropdownBase;

public:
    ~CheckboxDropdownCellWidget();
    CheckboxDropdownCellWidget(QWidget& parent, CheckboxDropdownBase& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

protected:
    CheckboxDropdownBase& m_value;
};



}
#endif
