/*  Max Lair AI Rental/Boss Matchup
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
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
            PA_THROW_StringException("Rental not found: " + rental);
        }
        auto iter1 = iter0->second.find(boss);
        if (iter1 == iter0->second.end()){
            PA_THROW_StringException("Boss not found: " + boss);
        }
        return iter1->second;
    }

private:
    MatchupDatabase(){
        QString path = RESOURCE_PATH() + QString::fromStdString("PokemonSwSh/MaxLair/boss_matchup_LUT.json");
        QJsonObject json = read_json_file(path).object();
        if (json.empty()){
            PA_THROW_FileException("Json is either empty or invalid.", path);
        }
        for (auto iter0 = json.begin(); iter0 != json.end(); ++iter0){
            std::map<std::string, double>& sub = map[iter0.key().toStdString()];
            QJsonObject obj = iter0.value().toObject();
            for (auto iter1 = obj.begin(); iter1 != obj.end(); ++iter1){
                sub[iter1.key().toStdString()] = iter1.value().toDouble();
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
