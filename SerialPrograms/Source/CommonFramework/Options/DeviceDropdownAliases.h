/*  Device Dropdown Aliases
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DeviceDropdownAliases_H
#define PokemonAutomation_DeviceDropdownAliases_H

#include <optional>
#include <string>
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StringOption.h"

namespace PokemonAutomation{

enum class DeviceAliasType{
    SERIAL,
    VIDEO,
    AUDIO,
};

class DeviceAliasRow : public EditableTableRow{
public:
    DeviceAliasRow(EditableTableOption& parent);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    EnumDropdownCell<DeviceAliasType> category;
    StringCell device_id;
    StringCell device_name;
    StringCell alias;
};

class DeviceDropdownAliases : public EditableTableOption_t<DeviceAliasRow>{
public:
    DeviceDropdownAliases();

    virtual std::vector<std::string> make_header() const override;
    void add_connected_devices();
    std::optional<std::string> lookup(DeviceAliasType category, const std::string& device_id) const;

private:
    void add_device(DeviceAliasType category, const std::string& id, const std::string& name);
};

class DeviceAliasOptions : public BatchOption{
public:
    DeviceAliasOptions();

    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

    void add_connected_devices();
    std::optional<std::string> lookup(DeviceAliasType category, const std::string& device_id) const;

private:
    DeviceDropdownAliases m_aliases;
};

//  Presentation only: never use this result for device selection or serialization.
//  Returns std::nullopt when no non-empty alias is configured.
//  Configured in Global Settings. Device IDs remain unchanged.
std::optional<std::string> device_dropdown_name(
    DeviceAliasType category,
    const std::string& device_id
);

}
#endif
