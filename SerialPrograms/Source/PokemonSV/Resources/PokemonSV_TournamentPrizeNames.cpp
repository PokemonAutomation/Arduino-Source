/*  Tournament Prize Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSV_TournamentPrizeNames.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


struct TournamentPrizeNameDatabase{
    TournamentPrizeNameDatabase();

    static const TournamentPrizeNameDatabase& instance(){
        static TournamentPrizeNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::vector<std::string> ordered_list;
    std::map<std::string, TournamentPrizeNames> database;
    std::map<std::string, std::string> reverse_lookup;
};
const std::string TournamentPrizeNameDatabase::NULL_SLUG;

TournamentPrizeNameDatabase::TournamentPrizeNameDatabase()
{
    // Load a list of tournament prize slugs in the desired order:
    // ["potion", "fresh-water", ... ]
    std::string path_slugs = RESOURCE_PATH() + "PokemonSV/AAT/TournamentPrizeList.json";
    JsonValue json_slugs = load_json_file(path_slugs);
    JsonArray& slugs = json_slugs.to_array_throw(path_slugs);

    // Load a map of tournament prize slugs to item names in all languages, e.g.:
    // {
    //      "potion": {
    //          "eng": "Potion",
    //          "deu": "Trank",
    //          ...
    //      },
    //      ....
    // }
    std::string path_disp = RESOURCE_PATH() + "PokemonSV/AAT/TournamentPrizeNameDisplay.json";
    JsonValue json_disp = load_json_file(path_disp);
    JsonObject& item_disp = json_disp.to_object_throw(path_disp);

    for (auto& item : slugs){
        std::string& slug = item.to_string_throw(path_slugs);

        JsonObject& auction_item_name_dict = item_disp.get_object_throw(slug, path_disp);
        std::string& display_name = auction_item_name_dict.get_string_throw("eng", path_disp);

        ordered_list.push_back(slug);
        database[std::move(slug)].m_display_name = std::move(display_name);
    }

    for (const auto& item : database){
        reverse_lookup[item.second.m_display_name] = item.first;
    }
}

const TournamentPrizeNames& get_tournament_prize_name(const std::string& slug){
    const std::map<std::string, TournamentPrizeNames>& database = TournamentPrizeNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Tournament prize slug not found in database: " + slug
        );
    }
    return iter->second;
}
const std::string& parse_tournament_prize_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = TournamentPrizeNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Tournament prize name not found in database: " + display_name
        );
    }
    return iter->second;
}
const std::string& parse_tournament_prize_name_nothrow(const std::string& display_name){
    const std::map<std::string, std::string>& database = TournamentPrizeNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return TournamentPrizeNameDatabase::NULL_SLUG;
    }
    return iter->second;
}


const std::vector<std::string>& TOURNAMENT_PRIZE_SLUGS(){
    return TournamentPrizeNameDatabase::instance().ordered_list;
}


}
}
}
