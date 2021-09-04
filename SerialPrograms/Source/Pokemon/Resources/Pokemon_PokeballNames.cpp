/*  Pokemon Pokeball Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Pokemon_PokeballNames.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokeballNameDatabase{
    PokeballNameDatabase();

    static const PokeballNameDatabase& instance(){
        static PokeballNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::vector<std::string> ordered_list;
    std::map<std::string, PokeballNames> database;
    std::map<QString, std::string> reverse_lookup;
};
const std::string PokeballNameDatabase::NULL_SLUG;

PokeballNameDatabase::PokeballNameDatabase()
    : ordered_list{
        "poke-ball",
        "great-ball",
        "ultra-ball",
        "master-ball",
        "premier-ball",
        "heal-ball",
        "net-ball",
        "nest-ball",
        "dive-ball",
        "dusk-ball",
        "timer-ball",
        "quick-ball",
        "repeat-ball",
        "luxury-ball",
        "fast-ball",
        "friend-ball",
        "lure-ball",
        "level-ball",
        "heavy-ball",
        "love-ball",
        "moon-ball",
        "dream-ball",
        "sport-ball",
        "safari-ball",
        "beast-ball",
    }
{
    database["poke-ball"    ].m_display_name = QString("Pok") + QChar(0xe9) + " Ball";
    database["great-ball"   ].m_display_name = "Great Ball";
    database["ultra-ball"   ].m_display_name = "Ultra Ball";
    database["master-ball"  ].m_display_name = "Master Ball";
    database["premier-ball" ].m_display_name = "Premier Ball";
    database["heal-ball"    ].m_display_name = "Heal Ball";
    database["net-ball"     ].m_display_name = "Net Ball";
    database["nest-ball"    ].m_display_name = "Nest Ball";
    database["dive-ball"    ].m_display_name = "Dive Ball";
    database["dusk-ball"    ].m_display_name = "Dusk Ball";
    database["timer-ball"   ].m_display_name = "Timer Ball";
    database["quick-ball"   ].m_display_name = "Quick Ball";
    database["repeat-ball"  ].m_display_name = "Repeat Ball";
    database["luxury-ball"  ].m_display_name = "Luxury Ball";
    database["fast-ball"    ].m_display_name = "Fast Ball";
    database["friend-ball"  ].m_display_name = "Friend Ball";
    database["lure-ball"    ].m_display_name = "Lure Ball";
    database["level-ball"   ].m_display_name = "Level Ball";
    database["heavy-ball"   ].m_display_name = "Heavy Ball";
    database["love-ball"    ].m_display_name = "Love Ball";
    database["moon-ball"    ].m_display_name = "Moon Ball";
    database["dream-ball"   ].m_display_name = "Dream Ball";
    database["sport-ball"   ].m_display_name = "Sport Ball";
    database["safari-ball"  ].m_display_name = "Safari Ball";
    database["beast-ball"   ].m_display_name = "Beast Ball";

    for (const auto& item : database){
        reverse_lookup[item.second.m_display_name] = item.first;
    }
}

const PokeballNames& get_pokeball_name(const std::string& slug){
    const std::map<std::string, PokeballNames>& database = PokeballNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokeball slug not found in database: " + slug);
    }
    return iter->second;
}
const std::string& parse_pokeball_name(const QString& display_name){
    const std::map<QString, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        PA_THROW_StringException("Pokeball name not found in database: " + display_name);
    }
    return iter->second;
}
const std::string& parse_pokeball_name_nothrow(const QString& display_name){
    const std::map<QString, std::string>& database = PokeballNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return PokeballNameDatabase::NULL_SLUG;
    }
    return iter->second;
}


const std::vector<std::string>& POKEBALL_SLUGS(){
    return PokeballNameDatabase::instance().ordered_list;
}


}
}
