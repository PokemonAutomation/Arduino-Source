/*  BDSP AutoStory - Game Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonBDSP_AutoStory_BDSPData.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


static const std::string RESOURCE_DIR = "PokemonBDSP/AutoStory/";


// ---------------------------------------------------------------------------
//  Shared parsing helpers
// ---------------------------------------------------------------------------

static PokemonType parse_type(const std::string& slug, const std::string& path){
    static const std::map<std::string, PokemonType> TABLE{
        {"normal",   PokemonType::NORMAL},
        {"fire",     PokemonType::FIRE},
        {"fighting", PokemonType::FIGHTING},
        {"water",    PokemonType::WATER},
        {"flying",   PokemonType::FLYING},
        {"grass",    PokemonType::GRASS},
        {"poison",   PokemonType::POISON},
        {"electric", PokemonType::ELECTRIC},
        {"ground",   PokemonType::GROUND},
        {"psychic",  PokemonType::PSYCHIC},
        {"rock",     PokemonType::ROCK},
        {"ice",      PokemonType::ICE},
        {"bug",      PokemonType::BUG},
        {"dragon",   PokemonType::DRAGON},
        {"ghost",    PokemonType::GHOST},
        {"dark",     PokemonType::DARK},
        {"steel",    PokemonType::STEEL},
        {"fairy",    PokemonType::FAIRY},
    };
    auto iter = TABLE.find(slug);
    if (iter == TABLE.end()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unknown type: " + slug, path);
    }
    return iter->second;
}

static MoveCategory parse_category(const std::string& slug, const std::string& path){
    if (slug == "physical") return MoveCategory::PHYSICAL;
    if (slug == "special")  return MoveCategory::SPECIAL;
    if (slug == "status")   return MoveCategory::STATUS;
    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unknown move category: " + slug, path);
}


// ---------------------------------------------------------------------------
//  Move database
// ---------------------------------------------------------------------------

struct BDSPMoveDatabase{
    std::map<std::string, BDSPMoveData> data;

    static const BDSPMoveDatabase& instance(){
        static BDSPMoveDatabase db;
        return db;
    }

    BDSPMoveDatabase(){
        std::string path = RESOURCE_PATH() + RESOURCE_DIR + "bdsp_move_data.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        for (auto& item : root){
            const std::string& slug = item.first;
            JsonObject& obj = item.second.to_object_throw(path);

            BDSPMoveData& move = data[slug];
            move.category   = parse_category(obj.get_string_throw("category", path), path);
            move.type       = parse_type(obj.get_string_throw("type", path), path);
            move.base_power = (uint8_t)obj.get_integer_throw("base_power", path);
            move.pp         = (uint8_t)obj.get_integer_throw("pp", path);
            move.spread     = obj.get_boolean_throw("spread", path);
            move.accuracy   = (uint8_t)obj.get_integer_default("accuracy", 100);
        }
    }
};

const BDSPMoveData& bdsp_move(const std::string& slug){
    const auto& db = BDSPMoveDatabase::instance().data;
    auto iter = db.find(slug);
    if (iter == db.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BDSP move slug: " + slug);
    }
    return iter->second;
}


// ---------------------------------------------------------------------------
//  Pokemon database
// ---------------------------------------------------------------------------

struct BDSPPokemonDatabase{
    std::map<std::string, BDSPPokemonData> data;

    static const BDSPPokemonDatabase& instance(){
        static BDSPPokemonDatabase db;
        return db;
    }

    BDSPPokemonDatabase(){
        std::string path = RESOURCE_PATH() + RESOURCE_DIR + "bdsp_pokemon_data.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        for (auto& item : root){
            const std::string& slug = item.first;
            JsonObject& obj = item.second.to_object_throw(path);

            BDSPPokemonData& mon = data[slug];
            mon.type[0] = PokemonType::NONE;
            mon.type[1] = PokemonType::NONE;
            {
                JsonArray& type_arr = obj.get_array_throw("type", path);
                if (type_arr.size() >= 1)
                    mon.type[0] = parse_type(type_arr[0].to_string_throw(path), path);
                if (type_arr.size() >= 2)
                    mon.type[1] = parse_type(type_arr[1].to_string_throw(path), path);
            }
            {
                JsonObject& stats = obj.get_object_throw("base_stats", path);
                mon.hp      = (uint8_t)stats.get_integer_throw("hp",      path);
                mon.attack  = (uint8_t)stats.get_integer_throw("attack",  path);
                mon.defense = (uint8_t)stats.get_integer_throw("defense", path);
                mon.spatk   = (uint8_t)stats.get_integer_throw("spatk",   path);
                mon.spdef   = (uint8_t)stats.get_integer_throw("spdef",   path);
                mon.speed   = (uint8_t)stats.get_integer_throw("speed",   path);
            }
        }
    }
};

const BDSPPokemonData& bdsp_pokemon(const std::string& slug){
    const auto& db = BDSPPokemonDatabase::instance().data;
    auto iter = db.find(slug);
    if (iter == db.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BDSP pokemon slug: " + slug);
    }
    return iter->second;
}


// ---------------------------------------------------------------------------
//  Trainer database
// ---------------------------------------------------------------------------

struct BDSPTrainerDatabase{
    std::map<std::string, BDSPTrainerData> data;

    static const BDSPTrainerDatabase& instance(){
        static BDSPTrainerDatabase db;
        return db;
    }

    BDSPTrainerDatabase(){
        std::string path = RESOURCE_PATH() + RESOURCE_DIR + "trainers.json";
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        for (auto& item : root){
            const std::string& battle_id = item.first;
            JsonObject& obj = item.second.to_object_throw(path);

            BDSPTrainerData& battle = data[battle_id];
            {
                const std::string& type_str = obj.get_string_throw("battle_type", path);
                if (type_str == "2v2")
                    battle.battle_type = BDSPBattleType::DOUBLES;
                else
                    battle.battle_type = BDSPBattleType::SINGLES;
            }
            {
                JsonArray& trainers_arr = obj.get_array_throw("trainers", path);
                for (auto& t_val : trainers_arr){
                    JsonObject& t_obj = t_val.to_object_throw(path);
                    BDSPTrainer trainer;
                    JsonArray& pkmn_arr = t_obj.get_array_throw("pokemon", path);
                    for (auto& p_val : pkmn_arr){
                        JsonObject& p_obj = p_val.to_object_throw(path);
                        BDSPTrainerPokemon tp;
                        tp.species = p_obj.get_string_throw("species", path);
                        tp.level   = (uint8_t)p_obj.get_integer_throw("level", path);

                        auto parse_stat_set = [&](BDSPStatSet& set, const char* key){
                            JsonObject* s = p_obj.get_object(key);
                            if (!s) return;
                            set.hp      = (uint8_t)s->get_integer_default("hp",      0);
                            set.attack  = (uint8_t)s->get_integer_default("attack",  0);
                            set.defense = (uint8_t)s->get_integer_default("defense", 0);
                            set.spatk   = (uint8_t)s->get_integer_default("spatk",   0);
                            set.spdef   = (uint8_t)s->get_integer_default("spdef",   0);
                            set.speed   = (uint8_t)s->get_integer_default("speed",   0);
                        };
                        parse_stat_set(tp.ivs, "ivs");
                        parse_stat_set(tp.evs, "evs");
                        tp.has_intimidate = p_obj.get_boolean_default("intimidate", false);

                        trainer.pokemon.push_back(std::move(tp));
                    }
                    battle.trainers.push_back(std::move(trainer));
                }
            }
        }
    }
};

const BDSPTrainerData& bdsp_trainer(const std::string& battle_id){
    const auto& db = BDSPTrainerDatabase::instance().data;
    auto iter = db.find(battle_id);
    if (iter == db.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BDSP battle ID: " + battle_id);
    }
    return iter->second;
}


}
}
}
