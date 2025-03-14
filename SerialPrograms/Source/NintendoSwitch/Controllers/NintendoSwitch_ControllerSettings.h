/*  Nintendo Switch Controller Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerSettings_H
#define PokemonAutomation_NintendoSwitch_ControllerSettings_H

#include <deque>
#include <mutex>
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/ColorOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{





class ControllerSettingsRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ~ControllerSettingsRow();
    ControllerSettingsRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void value_changed(void* object) override;

private:
    StringCell name;
    EnumDropdownCell<ControllerType> controller;
    ColorCell button_color;
    ColorCell body_color;
    ColorCell left_grip;
    ColorCell right_grip;
    StringSelectCell official_color;

private:
    std::atomic<size_t> m_pending_official_load;
};


class ControllerSettingsTable : public EditableTableOption_t<ControllerSettingsRow>{
public:
    ControllerSettingsTable();
    virtual std::vector<std::string> make_header() const override;



};


}
}
#endif
