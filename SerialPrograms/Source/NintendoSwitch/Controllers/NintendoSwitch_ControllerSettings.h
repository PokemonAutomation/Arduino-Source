/*  Nintendo Switch Controller Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ControllerSettings_H
#define PokemonAutomation_NintendoSwitch_ControllerSettings_H

#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/ColorOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/MacAddressOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



struct ControllerProfile{
    uint32_t button_color;
    uint32_t body_color;
    uint32_t left_grip;
    uint32_t right_grip;
    std::string official_name;
};

class ControllerSettingsRow : public EditableTableRow, public ConfigOption::Listener{
public:
    ~ControllerSettingsRow();
    ControllerSettingsRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual void on_config_value_changed(void* object) override;

    operator ControllerProfile() const{
        return {
            button_color,
            body_color,
            left_grip,
            right_grip,
            official_color.slug(),
        };
    }
    void set_profile(const ControllerProfile& profile){
        button_color.set(profile.button_color);
        body_color.set(profile.body_color);
        left_grip.set(profile.left_grip);
        right_grip.set(profile.right_grip);
        official_color.set_by_slug(profile.official_name);
    }

public:
    StringCell name;
    MacAddressCell controller_mac_address;
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

    static ControllerProfile random_profile(ControllerType controller);
    ControllerProfile get_or_make_profile(
        const uint8_t mac_address[6],
        const std::string& name,
        ControllerType controller
    );

};


}
}
#endif
