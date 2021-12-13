/*  Pokemon Sword/Shield Max Lair Rentals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh_MaxLairDatabase.h"

#include <map>
#include <algorithm>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




struct MaxLairSlugsDatabase{
    std::map<std::string, MaxLairSlugs> m_slugs;

    static MaxLairSlugsDatabase& instance(){
        static MaxLairSlugsDatabase data;
        return data;
    }

    MaxLairSlugsDatabase(){
        QString path = RESOURCE_PATH() + QString::fromStdString("PokemonSwSh/MaxLairSlugMap.json");
        QJsonObject json = read_json_file(path).object();
        if (json.empty()){
            PA_THROW_FileException("Json is either empty or invalid.", path);
        }

        for (auto iter = json.begin(); iter != json.end(); ++iter){
            std::string maxlair_slug = iter.key().toStdString();
            QJsonObject obj = iter.value().toObject();
            MaxLairSlugs slugs;
            for (const auto& item : obj["OCR"].toArray()){
                std::string slug = item.toString().toStdString();
                if (!slugs.name_slug.empty()){
                    PA_THROW_ParseException("Multiple names specified for MaxLair slug.");
                }
                slugs.name_slug = std::move(slug);
            }
            for (const auto& item : obj["Sprite"].toArray()){
                std::string slug = item.toString().toStdString();
                slugs.sprite_slugs.insert(std::move(slug));
            }
            m_slugs[maxlair_slug] = std::move(slugs);
        }
    }
};



const std::map<std::string, MaxLairSlugs>& maxlair_slugs(){
    return MaxLairSlugsDatabase::instance().m_slugs;
}
const MaxLairSlugs& get_maxlair_slugs(const std::string& slug){
    const MaxLairSlugsDatabase& database = MaxLairSlugsDatabase::instance();
    auto iter = database.m_slugs.find(slug);
    if (iter == database.m_slugs.end()){
        PA_THROW_StringException("Invalid Max Lair slug: " + slug);
    }
    return iter->second;
}





MoveCategory parse_category_slug(const std::string& slug){
    static const std::map<std::string, MoveCategory> database{
        {"status",      MoveCategory::STATUS},
        {"physical",    MoveCategory::PHYSICAL},
        {"special",     MoveCategory::SPECIAL},
    };
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Invalid Move Category: " + slug);
    }
    return iter->second;
}

PokemonType parse_type_slug(const std::string& slug){
    static const std::map<std::string, PokemonType> database{
        {"normal",      PokemonType::NORMAL},
        {"fire",        PokemonType::FIRE},
        {"fighting",    PokemonType::FIGHTING},
        {"water",       PokemonType::WATER},
        {"flying",      PokemonType::FLYING},
        {"grass",       PokemonType::GRASS},
        {"poison",      PokemonType::POISON},
        {"electric",    PokemonType::ELECTRIC},
        {"ground",      PokemonType::GROUND},
        {"psychic",     PokemonType::PSYCHIC},
        {"rock",        PokemonType::ROCK},
        {"ice",         PokemonType::ICE},
        {"bug",         PokemonType::BUG,},
        {"dragon",      PokemonType::DRAGON},
        {"ghost",       PokemonType::GHOST},
        {"dark",        PokemonType::DARK},
        {"steel",       PokemonType::STEEL},
        {"fairy",       PokemonType::FAIRY},
    };
    auto iter = database.find(slug);
    if (iter == database.end()){
        return PokemonType::NONE;
    }
    return iter->second;
}

MaxLairMove parse_move(const QJsonObject& obj){
    MaxLairMove move;
    move.slug = obj["move"].toString().toStdString();
    move.category = parse_category_slug(obj["category"].toString().toStdString());
    move.type = parse_type_slug(obj["type"].toString().toStdString());
    move.base_power = obj["base_power"].toInt();
    move.accuracy = obj["accuracy"].toDouble();
    move.PP = obj["PP"].toInt();
    move.spread = obj["spread"].toBool();
    move.correction_factor = obj["correction_factor"].toDouble();
    move.effective_power = obj["effective_power"].toDouble();
    return move;
}


std::map<std::string, MaxLairMon> build_maxlair_mon_database(const std::string& path){
    QJsonObject json = read_json_file(
        RESOURCE_PATH() + QString::fromStdString(path)
    ).object();

    std::map<std::string, MaxLairMon> database;

    for (auto iter = json.begin(); iter != json.end(); ++iter){
        std::string slug = iter.key().toStdString();
        QJsonObject obj = iter.value().toObject();
        MaxLairMon& mon = database[slug];
        mon.species = slug;
        mon.type[0] = PokemonType::NONE;
        mon.type[1] = PokemonType::NONE;
        {
            QJsonArray array = obj["type"].toArray();
            if (array.size() >= 1){
                mon.type[0] = parse_type_slug(array[0].toString().toStdString());
            }
            if (array.size() >= 2){
                mon.type[1] = parse_type_slug(array[1].toString().toStdString());
            }
        }
        mon.ability = obj["ability"].toString().toStdString();
        {
            QJsonArray array = obj["base_stats"].toArray();
            if (array.size() != 6){
                PA_THROW_StringException("Base stats should contain 6 elements.");
            }
            for (int c = 0; c < 6; c++){
                mon.base_stats[c] = (uint8_t)array[c].toInt();
            }
        }
        {
            QJsonArray array = obj["moves"].toArray();
            int stop = std::min(5, (int)array.size());
            for (int c = 0; c < stop; c++){
                mon.moves[c] = parse_move(array[c].toObject());
            }
        }
        {
            QJsonArray array = obj["max_moves"].toArray();
            int stop = std::min(5, (int)array.size());
            for (int c = 0; c < stop; c++){
                mon.max_moves[c] = parse_move(array[c].toObject());
            }
        }
    }

    return database;
}




struct MaxLairDatabase{
    std::map<std::string, MaxLairMon> m_rentals;
    std::map<std::string, MaxLairMon> m_bosses;

    std::map<size_t, std::string> m_bosses_by_dex;

    static MaxLairDatabase& instance(){
        static MaxLairDatabase data;
        return data;
    }

    MaxLairDatabase()
        : m_rentals(build_maxlair_mon_database("PokemonSwSh/MaxLairRentals.json"))
        , m_bosses(build_maxlair_mon_database("PokemonSwSh/MaxLairBosses.json"))
    {
        const std::map<std::string, size_t>& national_dex = SLUGS_TO_NATIONAL_DEX();

        for (const auto& item : m_bosses){
            const MaxLairSlugs& slugs = get_maxlair_slugs(item.first);
            auto iter = national_dex.find(slugs.name_slug);
            if (iter == national_dex.end()){
                PA_THROW_StringException("Slug not found in national dex: " + slugs.name_slug);
            }
            m_bosses_by_dex[iter->second] = item.first;
        }

#if 0
        for (const auto& item : m_bosses_by_dex){
            const MaxLairSlugs& slugs = get_maxlair_slugs(item.second);
            cout << item.first << " : "
                 << item.second << " : "
                 << get_pokemon_name(slugs.name_slug).display_name().toStdString() << endl;
        }
#endif
    }
};




const std::map<size_t, std::string>& all_bosses_by_dex(){
    const MaxLairDatabase& database = MaxLairDatabase::instance();
    return database.m_bosses_by_dex;
}
bool is_boss(const std::string& slug){
    const MaxLairDatabase& database = MaxLairDatabase::instance();
    auto iter = database.m_bosses.find(slug);
    return iter != database.m_bosses.end();
}
const MaxLairMon& get_maxlair_mon(const std::string& slug){
    const MaxLairDatabase& database = MaxLairDatabase::instance();
    auto iter0 = database.m_rentals.find(slug);
    if (iter0 != database.m_rentals.end()){
        return iter0->second;
    }
    auto iter1 = database.m_bosses.find(slug);
    if (iter1 != database.m_bosses.end()){
        return iter1->second;
    }
    PA_THROW_StringException("Invalid Species Slug: " + slug);
}
const MaxLairMon* get_maxlair_mon_nothrow(const std::string& slug){
    const MaxLairDatabase& database = MaxLairDatabase::instance();
    auto iter0 = database.m_rentals.find(slug);
    if (iter0 != database.m_rentals.end()){
        return &iter0->second;
    }
    auto iter1 = database.m_bosses.find(slug);
    if (iter1 != database.m_bosses.end()){
        return &iter1->second;
    }
    return nullptr;
}





}
}
}
}
