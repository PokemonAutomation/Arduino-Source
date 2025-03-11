/*  Pokemon Sword/Shield Max Lair Rentals
 *
 *  From: https://github.com/PokemonAutomation/
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
        std::string path = RESOURCE_PATH() + "PokemonSwSh/MaxLairSlugMap.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        for (auto& item0 : root){
            const std::string& maxlair_slug = item0.first;
            JsonObject& obj = item0.second.to_object_throw(path);
            MaxLairSlugs slugs;
            for (auto& item1 : obj.get_array_throw("OCR", path)){
                std::string& slug = item1.to_string_throw(path);
                if (!slugs.name_slug.empty()){
                    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Multiple names specified for MaxLair slug.", std::move(path));
                }
                slugs.name_slug = std::move(slug);
            }
            for (auto& item1 : obj.get_array_throw("Sprite", path)){
                std::string& slug = item1.to_string_throw(path);
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

MaxLairMove parse_move(JsonObject&& obj, const std::string& path){
    MaxLairMove move;
    move.slug               = obj.get_string_throw("move", path);
    move.category           = parse_category_slug(obj.get_string_throw("category", path));
    move.type               = parse_type_slug(obj.get_string_throw("type", path));
    move.base_power         = (uint8_t)obj.get_integer_throw("base_power", path);
    move.accuracy           = obj.get_double_throw("accuracy", path);
    move.PP                 = (uint8_t)obj.get_integer_throw("PP", path);
    move.spread             = obj.get_boolean_throw("spread", path);
    move.correction_factor  = obj.get_double_throw("correction_factor", path);
    move.effective_power    = obj.get_double_throw("effective_power", path);
    return move;
}


std::map<std::string, MaxLairMon> build_maxlair_mon_database(const std::string& path){
    std::string filepath = RESOURCE_PATH() + path;
    JsonValue json = load_json_file(filepath);
    JsonObject& root = json.to_object_throw(filepath);

    std::map<std::string, MaxLairMon> database;

    for (auto& item : root){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.to_object_throw(filepath);

        MaxLairMon& mon = database[slug];
        mon.species = slug;
        mon.type[0] = PokemonType::NONE;
        mon.type[1] = PokemonType::NONE;
        {
            JsonArray& array = obj.get_array_throw("type", filepath);
            if (array.size() >= 1){
                std::string& str = array[0].to_string_throw(filepath);
                mon.type[0] = parse_type_slug(str);
            }
            if (array.size() >= 2){
                std::string& str = array[1].to_string_throw(filepath);
                mon.type[1] = parse_type_slug(str);
            }
        }
        {
            std::string& str = obj.get_string_throw("ability", filepath);
            mon.ability = std::move(str);
        }
        {
            JsonArray& array = obj.get_array_throw("base_stats", filepath);
            if (array.size() != 6){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Base stats should contain 6 elements: " + slug, std::move(filepath));
            }
            for (int c = 0; c < 6; c++){
                mon.base_stats[c] = (uint8_t)array[c].to_integer_throw(filepath);
            }
        }
        {
            JsonArray& array = obj.get_array_throw("moves", filepath);
            size_t stop = std::min<size_t>(5, array.size());
            for (size_t c = 0; c < stop; c++){
                JsonObject& move = array[c].to_object_throw(filepath);
                mon.moves[c] = parse_move(std::move(move), filepath);
            }
        }
        {
            JsonArray& array = obj.get_array_throw("max_moves", filepath);
            size_t stop = std::min<size_t>(5, array.size());
            for (size_t c = 0; c < stop; c++){
                JsonObject& move = array[c].to_object_throw(filepath);
                mon.max_moves[c] = parse_move(std::move(move), filepath);
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
