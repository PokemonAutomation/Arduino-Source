/*  Encounters Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "PokemonFRLG_EncountersDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


EncountersDatabase::EncountersDatabase(const char* json_path){
    std::string path = RESOURCE_PATH() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    for (auto& item : root){
        const std::string& encounter_type = item.first;
        JsonObject& type_obj = item.second.to_object_throw(path);
        std::map<std::string, std::vector<AdvEncounterSlot>> loc_map;
        for (auto& loc_item : type_obj){
            const std::string& loc_name = loc_item.first;
            JsonArray& encs_arr = loc_item.second.to_array_throw(path);
            std::vector<AdvEncounterSlot> encounter_slots;
            for (auto& enc_item : encs_arr){
                JsonObject& enc_obj = enc_item.to_object_throw(path);
                std::string species = enc_obj.get_string_throw("species", path);
                uint8_t minlevel = uint8_t(enc_obj.get_integer_throw("minLevel", path));
                uint8_t maxlevel = uint8_t(enc_obj.get_integer_throw("maxLevel", path));
                AdvEncounterSlot slot = { species, minlevel, maxlevel };
                encounter_slots.emplace_back(slot);
            }
            loc_map.emplace(loc_name, encounter_slots);
        }
        m_database.emplace(encounter_type, loc_map);
    }
}

const std::map<std::string, std::vector<AdvEncounterSlot>>& EncountersDatabase::get_throw(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found in database: " + slug);
    }
    return iter->second;
}
const std::map<std::string, std::vector<AdvEncounterSlot>>* EncountersDatabase::get_nothrow(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
}
}