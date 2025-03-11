/*  Max Lair AI Rental/Boss Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh_MaxLair_AI_RentalBossMatchup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



struct MatchupDatabase{
    std::map<std::string, std::map<std::string, double>> map;

    static const MatchupDatabase& instance(){
        static MatchupDatabase database;
        return database;
    }

    double get(const std::string& rental, const std::string& boss) const{
        auto iter0 = map.find(rental);
        if (iter0 == map.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Rental not found: " + rental);
        }
        auto iter1 = iter0->second.find(boss);
        if (iter1 == iter0->second.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Boss not found: " + rental);
        }
        return iter1->second;
    }

private:
    MatchupDatabase(){
        std::string path = RESOURCE_PATH() + "PokemonSwSh/MaxLair/boss_matchup_LUT.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);
        for (auto& item0 : root){
            std::map<std::string, double>& sub = map[item0.first];
            JsonObject& obj = item0.second.to_object_throw(path);
            for (auto& item1 : obj){
                sub[item1.first] = item1.second.to_double_throw(path);
            }
        }
    }
};

double rental_vs_boss_matchup(const std::string& rental, const std::string& boss){
    return MatchupDatabase::instance().get(rental, boss);
}
double rental_vs_boss_matchup(const std::string& rental, const std::vector<std::string>& bosses){
    using namespace papkmnlib;

    double score = 0;
    if (bosses.empty()){
        const auto& all_bosses = all_boss_pokemon();
        for (const auto& boss : all_bosses){
            score += rental_vs_boss_matchup(rental, boss.second.name());
        }
        score /= bosses.size();
    }else{
        for (const std::string& boss : bosses){
            score += rental_vs_boss_matchup(rental, boss);
        }
        score /= bosses.size();
    }
    return score;
}





}
}
}
}
