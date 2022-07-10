/*  Pokemon Sword/Shield Max Lair Rentals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh_MaxLairDatabase.h"

#include "Common/Qt/QtJsonTools.h"

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
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Json is either empty or invalid.",
                path.toStdString()
            );
        }

        for (auto iter = json.begin(); iter != json.end(); ++iter){
            std::string maxlair_slug = iter.key().toStdString();
            QJsonObject obj = iter.value().toObject();
            MaxLairSlugs slugs;
            for (const auto& item : obj["OCR"].toArray()){
                std::string slug = item.toString().toStdString();
                if (!slugs.name_slug.empty()){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Multiple names specified for MaxLair slug.", path.toStdString());
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
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Max Lair slug: " + slug);
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
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Move Category: " + slug);
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

MaxLairMove parse_move(const JsonObject& obj){
    MaxLairMove move;
    move.slug = obj.to_string("move");
    move.category = parse_category_slug(obj.to_string("category"));
    move.type = parse_type_slug(obj.to_string("type"));
    move.base_power = obj.to_integer("base_power");
    move.accuracy = obj.to_double("accuracy");
    move.PP = obj.to_integer("PP");
    move.spread = obj.to_boolean("spread");
    move.correction_factor = obj.to_double("correction_factor");
    move.effective_power = obj.to_double("effective_power");
    return move;
}


std::map<std::string, MaxLairMon> build_maxlair_mon_database(const std::string& path){
    std::string filepath = RESOURCE_PATH().toStdString() + path;
    JsonValue json = load_json_file(filepath);
    JsonObject* root = json.get_object();
    if (root == nullptr){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load resource.", std::move(path));
    }

    std::map<std::string, MaxLairMon> database;

    for (auto& item : *root){
        const std::string& slug = item.first;
        if (slug.empty()){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected non-empty string for Pokemon slug.", std::move(path));
        }

        JsonObject* obj = item.second.get_object();
        if (obj == nullptr){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "No data for Pokemon slug: " + slug, std::move(path));
        }

        MaxLairMon& mon = database[slug];
        mon.species = slug;
        mon.type[0] = PokemonType::NONE;
        mon.type[1] = PokemonType::NONE;
        {
            JsonArray* array = obj->get_array("type");
            if (array == nullptr){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Missing type: " + slug, std::move(path));
            }
            if (array->size() >= 1){
                std::string* str = (*array)[0].get_string();
                if (str == nullptr){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected string for type: " + slug, std::move(path));
                }
                mon.type[0] = parse_type_slug(*str);
            }
            if (array->size() >= 2){
                std::string* str = (*array)[1].get_string();
                if (str == nullptr){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected string for type: " + slug, std::move(path));
                }
                mon.type[1] = parse_type_slug(*str);
            }
        }
        {
            std::string* str = obj->get_string("ability");
            if (str != nullptr){
                mon.ability = *str;
            }
        }
        {
            JsonArray* array = obj->get_array("base_stats");
            if (array == nullptr || array->size() != 6){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Base stats should contain 6 elements: " + slug, std::move(path));
            }
            for (int c = 0; c < 6; c++){
                if (!(*array)[c].read_integer(mon.base_stats[c])){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "IVs should be integers: " + slug, std::move(path));
                }
            }
        }
        {
            JsonArray* array = obj->get_array("moves");
            if (array == nullptr){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "No moves found: " + slug, std::move(path));
            }
            size_t stop = std::min<size_t>(5, array->size());
            for (size_t c = 0; c < stop; c++){
                JsonObject* move = (*array)[c].get_object();
                if (move == nullptr){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected object for moves: " + slug, std::move(path));
                }
                mon.moves[c] = parse_move(*move);
            }
        }
        {
            JsonArray* array = obj->get_array("max_moves");
            if (array == nullptr){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "No max moves found: " + slug, std::move(path));
            }
            size_t stop = std::min<size_t>(5, array->size());
            for (size_t c = 0; c < stop; c++){
                JsonObject* move = (*array)[c].get_object();
                if (move == nullptr){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected object for max moves: " + slug, std::move(path));
                }
                mon.max_moves[c] = parse_move(*move);
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
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found in national dex: " + slugs.name_slug);
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
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Species Slug: " + slug);
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
