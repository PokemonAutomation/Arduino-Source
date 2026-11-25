/*  RNG Stats Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "PokemonFRLG_RngStatsDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


RngStatsDatabase::RngStatsDatabase(const char* json_path){
    std::string path = RESOURCE_PATH() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    for (auto& item : root){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.to_object_throw(path);
        std::vector<uint8_t> readstats;
        JsonArray& statsarr = obj.get_array_throw("baseStats", path);
        for (auto& stat : statsarr){
            auto statval = stat.to_integer_throw();
            readstats.emplace_back(uint8_t(statval));
        }
        if (readstats.size() != 6){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid length for base stats.", path);
        }
        BaseStats base_stats = { readstats[0], readstats[1], readstats[2], readstats[3], readstats[4], readstats[5] };

        int16_t gender_threshold = int16_t(obj.get_integer_throw("genderThreshold", path));

        std::vector<std::string> abilities;
        const JsonArray* abilities_arr = obj.get_array("abilities");
        if (abilities_arr != nullptr){
            for (const auto& ability : *abilities_arr){
                abilities.emplace_back(ability.to_string_throw(path));
            }
            if (abilities.size() > 2){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Too many abilities for " + slug + ".", path);
            }
        }

        RngStats rng_stats = { base_stats, gender_threshold, std::move(abilities) };

        m_database.emplace(
            slug,
            rng_stats
        );
    }
}

const RngStatsDatabase& RngStatsDatabase::instance(){
    static const RngStatsDatabase database("PokemonFRLG/BaseStats.json");
    return database;
}

const RngStats& RngStatsDatabase::get_throw(const std::string& slug) const{
    std::string sl = slug;
    if (slug.find("unown") != std::string::npos){
        sl = "unown";
    }
    auto iter = m_database.find(sl);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found in database: " + sl);
    }
    return iter->second;
}
const RngStats* RngStatsDatabase::get_nothrow(const std::string& slug) const{
    std::string sl = slug;
    if (slug.find("unown") != std::string::npos){
        sl = "unown";
    }
    auto iter = m_database.find(sl);
    if (iter == m_database.end()){
        return nullptr;
    }
    return &iter->second;
}

std::string ability_name(const RngStats* stats, AdvAbility ability){
    size_t slot;
    switch (ability){
    case AdvAbility::Zero:
        slot = 0;
        break;
    case AdvAbility::One:
        slot = 1;
        break;
    default:
        return "-";
    }
    if (stats == nullptr || stats->abilities.empty()){
        return "-";
    }
    // Species with only one ability use it regardless of the PID's ability bit.
    return stats->abilities[slot < stats->abilities.size() ? slot : 0];
}

std::string gender_name(const RngStats* stats, uint8_t gender_value){
    if (stats == nullptr){
        return "-";
    }
    return gender_to_string(gender_from_gender_value(gender_value, stats->gender_threshold));
}



}
}
}
