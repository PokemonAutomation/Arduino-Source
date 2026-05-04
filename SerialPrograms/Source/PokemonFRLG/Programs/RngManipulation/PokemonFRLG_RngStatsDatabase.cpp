/*  RNG Stats Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
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

        RngStats rng_stats = { base_stats, gender_threshold };

        m_database.emplace(
            slug,
            rng_stats
        );
    }
}

const RngStats& RngStatsDatabase::get_throw(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found in database: " + slug);
    }
    return iter->second;
}
const RngStats* RngStatsDatabase::get_nothrow(const std::string& slug) const{
    auto iter = m_database.find(slug);
    if (iter == m_database.end()){
        return nullptr;
    }
    return &iter->second;
}



}
}
}