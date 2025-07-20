/*  Daily Highlight Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"
#include "PokemonSwSh_DailyHighlightDatabase.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {



DailyHighlightDatabase::DailyHighlightDatabase(const char* resource_path) {
    std::string filepath = RESOURCE_PATH() + resource_path;
    JsonValue json = load_json_file(filepath);
    JsonObject& root = json.to_object_throw(filepath);

    for (auto& item : root) {
        const std::string& slug = item.first;
        JsonObject& obj = item.second.to_object_throw(filepath);

        uint16_t min = (uint16_t)obj.get_integer_throw("min", filepath);
        uint16_t max = (uint16_t)obj.get_integer_throw("max", filepath);
        m_slug_to_range[slug] = std::pair<uint16_t, uint16_t>(min, max);

        std::string display_name = obj.get_string_throw("display_name", filepath);
        StringSelectEntry string_select_entry(slug, display_name);
        m_stringselect_database.add_entry(string_select_entry);
    }
}

std::pair<uint16_t, uint16_t> DailyHighlightDatabase::get_range_for_slug(const std::string& slug) const {
    auto iter = m_slug_to_range.find(slug);
    if (iter == m_slug_to_range.end()) {
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            std::string("Invalid daily highlight slug: " + slug)
        );
    }
    return iter->second;
}



}
}
}
