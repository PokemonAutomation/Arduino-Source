/*  Device Dropdown Aliases
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DeviceDropdownAliases_H
#define PokemonAutomation_DeviceDropdownAliases_H

#include <optional>
#include <string>

namespace PokemonAutomation{

//  Presentation only: never use this result for device selection or serialization.
//  Returns std::nullopt when no non-empty alias is configured.
//  Loaded once from SETTINGS_PATH()/deviceAliases.json. Restart to reload edits.
std::optional<std::string> device_dropdown_name(
    const std::string& category,
    const std::string& device_id
);

}
#endif
