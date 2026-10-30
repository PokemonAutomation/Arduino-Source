/*  Device Dropdown Aliases
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "DeviceDropdownAliases.h"

namespace PokemonAutomation{
namespace{

JsonObject load_device_aliases(){
    std::string contents;
    if (!file_to_string(SETTINGS_PATH() + "deviceAliases.json", contents)){
        return JsonObject();
    }
    JsonValue json = parse_json(contents);
    JsonObject* object = json.to_object();
    if (object != nullptr){
        return std::move(*object);
    }
    return JsonObject();
}

}

std::optional<std::string> device_dropdown_name(
    const std::string& category,
    const std::string& device_id
){
    static const JsonObject aliases = load_device_aliases();
    const JsonObject* group = aliases.get_object(category);
    if (group == nullptr){
        return std::nullopt;
    }
    const std::string* alias = group->get_string(device_id);
    if (alias == nullptr || alias->empty()){
        return std::nullopt;
    }
    return *alias;
}

}
