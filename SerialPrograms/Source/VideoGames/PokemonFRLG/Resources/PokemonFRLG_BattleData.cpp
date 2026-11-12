/*  Battle Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "PokemonFRLG_BattleData.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using Pokemon::PokemonType;


bool is_special_type(PokemonType type){
    switch (type){
    case PokemonType::FIRE:
    case PokemonType::WATER:
    case PokemonType::GRASS:
    case PokemonType::ELECTRIC:
    case PokemonType::PSYCHIC:
    case PokemonType::ICE:
    case PokemonType::DRAGON:
    case PokemonType::DARK:
        return true;
    default:
        return false;
    }
}


namespace{

struct TypeMatchup{
    PokemonType attack;
    PokemonType defend;
    double multiplier;
};

//  gTypeEffectiveness in the decomp's src/battle_main.c. Pairs not listed
//  are 1x.
const TypeMatchup TYPE_CHART[] = {
    {PokemonType::NORMAL,   PokemonType::ROCK,      0.5},
    {PokemonType::NORMAL,   PokemonType::STEEL,     0.5},
    {PokemonType::NORMAL,   PokemonType::GHOST,     0.0},
    {PokemonType::FIRE,     PokemonType::FIRE,      0.5},
    {PokemonType::FIRE,     PokemonType::WATER,     0.5},
    {PokemonType::FIRE,     PokemonType::GRASS,     2.0},
    {PokemonType::FIRE,     PokemonType::ICE,       2.0},
    {PokemonType::FIRE,     PokemonType::BUG,       2.0},
    {PokemonType::FIRE,     PokemonType::ROCK,      0.5},
    {PokemonType::FIRE,     PokemonType::DRAGON,    0.5},
    {PokemonType::FIRE,     PokemonType::STEEL,     2.0},
    {PokemonType::WATER,    PokemonType::FIRE,      2.0},
    {PokemonType::WATER,    PokemonType::WATER,     0.5},
    {PokemonType::WATER,    PokemonType::GRASS,     0.5},
    {PokemonType::WATER,    PokemonType::GROUND,    2.0},
    {PokemonType::WATER,    PokemonType::ROCK,      2.0},
    {PokemonType::WATER,    PokemonType::DRAGON,    0.5},
    {PokemonType::ELECTRIC, PokemonType::WATER,     2.0},
    {PokemonType::ELECTRIC, PokemonType::ELECTRIC,  0.5},
    {PokemonType::ELECTRIC, PokemonType::GRASS,     0.5},
    {PokemonType::ELECTRIC, PokemonType::GROUND,    0.0},
    {PokemonType::ELECTRIC, PokemonType::FLYING,    2.0},
    {PokemonType::ELECTRIC, PokemonType::DRAGON,    0.5},
    {PokemonType::GRASS,    PokemonType::FIRE,      0.5},
    {PokemonType::GRASS,    PokemonType::WATER,     2.0},
    {PokemonType::GRASS,    PokemonType::GRASS,     0.5},
    {PokemonType::GRASS,    PokemonType::POISON,    0.5},
    {PokemonType::GRASS,    PokemonType::GROUND,    2.0},
    {PokemonType::GRASS,    PokemonType::FLYING,    0.5},
    {PokemonType::GRASS,    PokemonType::BUG,       0.5},
    {PokemonType::GRASS,    PokemonType::ROCK,      2.0},
    {PokemonType::GRASS,    PokemonType::DRAGON,    0.5},
    {PokemonType::GRASS,    PokemonType::STEEL,     0.5},
    {PokemonType::ICE,      PokemonType::WATER,     0.5},
    {PokemonType::ICE,      PokemonType::GRASS,     2.0},
    {PokemonType::ICE,      PokemonType::ICE,       0.5},
    {PokemonType::ICE,      PokemonType::GROUND,    2.0},
    {PokemonType::ICE,      PokemonType::FLYING,    2.0},
    {PokemonType::ICE,      PokemonType::DRAGON,    2.0},
    {PokemonType::ICE,      PokemonType::STEEL,     0.5},
    {PokemonType::ICE,      PokemonType::FIRE,      0.5},
    {PokemonType::FIGHTING, PokemonType::NORMAL,    2.0},
    {PokemonType::FIGHTING, PokemonType::ICE,       2.0},
    {PokemonType::FIGHTING, PokemonType::POISON,    0.5},
    {PokemonType::FIGHTING, PokemonType::FLYING,    0.5},
    {PokemonType::FIGHTING, PokemonType::PSYCHIC,   0.5},
    {PokemonType::FIGHTING, PokemonType::BUG,       0.5},
    {PokemonType::FIGHTING, PokemonType::ROCK,      2.0},
    {PokemonType::FIGHTING, PokemonType::DARK,      2.0},
    {PokemonType::FIGHTING, PokemonType::STEEL,     2.0},
    {PokemonType::FIGHTING, PokemonType::GHOST,     0.0},
    {PokemonType::POISON,   PokemonType::GRASS,     2.0},
    {PokemonType::POISON,   PokemonType::POISON,    0.5},
    {PokemonType::POISON,   PokemonType::GROUND,    0.5},
    {PokemonType::POISON,   PokemonType::ROCK,      0.5},
    {PokemonType::POISON,   PokemonType::GHOST,     0.5},
    {PokemonType::POISON,   PokemonType::STEEL,     0.0},
    {PokemonType::GROUND,   PokemonType::FIRE,      2.0},
    {PokemonType::GROUND,   PokemonType::ELECTRIC,  2.0},
    {PokemonType::GROUND,   PokemonType::GRASS,     0.5},
    {PokemonType::GROUND,   PokemonType::POISON,    2.0},
    {PokemonType::GROUND,   PokemonType::FLYING,    0.0},
    {PokemonType::GROUND,   PokemonType::BUG,       0.5},
    {PokemonType::GROUND,   PokemonType::ROCK,      2.0},
    {PokemonType::GROUND,   PokemonType::STEEL,     2.0},
    {PokemonType::FLYING,   PokemonType::ELECTRIC,  0.5},
    {PokemonType::FLYING,   PokemonType::GRASS,     2.0},
    {PokemonType::FLYING,   PokemonType::FIGHTING,  2.0},
    {PokemonType::FLYING,   PokemonType::BUG,       2.0},
    {PokemonType::FLYING,   PokemonType::ROCK,      0.5},
    {PokemonType::FLYING,   PokemonType::STEEL,     0.5},
    {PokemonType::PSYCHIC,  PokemonType::FIGHTING,  2.0},
    {PokemonType::PSYCHIC,  PokemonType::POISON,    2.0},
    {PokemonType::PSYCHIC,  PokemonType::PSYCHIC,   0.5},
    {PokemonType::PSYCHIC,  PokemonType::DARK,      0.0},
    {PokemonType::PSYCHIC,  PokemonType::STEEL,     0.5},
    {PokemonType::BUG,      PokemonType::FIRE,      0.5},
    {PokemonType::BUG,      PokemonType::GRASS,     2.0},
    {PokemonType::BUG,      PokemonType::FIGHTING,  0.5},
    {PokemonType::BUG,      PokemonType::POISON,    0.5},
    {PokemonType::BUG,      PokemonType::FLYING,    0.5},
    {PokemonType::BUG,      PokemonType::PSYCHIC,   2.0},
    {PokemonType::BUG,      PokemonType::GHOST,     0.5},
    {PokemonType::BUG,      PokemonType::DARK,      2.0},
    {PokemonType::BUG,      PokemonType::STEEL,     0.5},
    {PokemonType::ROCK,     PokemonType::FIRE,      2.0},
    {PokemonType::ROCK,     PokemonType::ICE,       2.0},
    {PokemonType::ROCK,     PokemonType::FIGHTING,  0.5},
    {PokemonType::ROCK,     PokemonType::GROUND,    0.5},
    {PokemonType::ROCK,     PokemonType::FLYING,    2.0},
    {PokemonType::ROCK,     PokemonType::BUG,       2.0},
    {PokemonType::ROCK,     PokemonType::STEEL,     0.5},
    {PokemonType::GHOST,    PokemonType::NORMAL,    0.0},
    {PokemonType::GHOST,    PokemonType::PSYCHIC,   2.0},
    {PokemonType::GHOST,    PokemonType::DARK,      0.5},
    {PokemonType::GHOST,    PokemonType::STEEL,     0.5},
    {PokemonType::GHOST,    PokemonType::GHOST,     2.0},
    {PokemonType::DRAGON,   PokemonType::DRAGON,    2.0},
    {PokemonType::DRAGON,   PokemonType::STEEL,     0.5},
    {PokemonType::DARK,     PokemonType::FIGHTING,  0.5},
    {PokemonType::DARK,     PokemonType::PSYCHIC,   2.0},
    {PokemonType::DARK,     PokemonType::GHOST,     2.0},
    {PokemonType::DARK,     PokemonType::DARK,      0.5},
    {PokemonType::DARK,     PokemonType::STEEL,     0.5},
    {PokemonType::STEEL,    PokemonType::FIRE,      0.5},
    {PokemonType::STEEL,    PokemonType::WATER,     0.5},
    {PokemonType::STEEL,    PokemonType::ELECTRIC,  0.5},
    {PokemonType::STEEL,    PokemonType::ICE,       2.0},
    {PokemonType::STEEL,    PokemonType::ROCK,      2.0},
    {PokemonType::STEEL,    PokemonType::STEEL,     0.5},
};


//  How a move's effect (from the decomp, e.g. EFFECT_MULTI_HIT is
//  "multi-hit") changes how its power counts.
MoveKind kind_of_effect(const std::string& effect){
    static const std::map<std::string, MoveKind> KINDS{
        {"multi-hit",           MoveKind::MULTI_HIT},
        {"double-hit",          MoveKind::DOUBLE_HIT},
        {"twineedle",           MoveKind::DOUBLE_HIT},
        {"triple-kick",         MoveKind::TRIPLE_KICK},
        {"recharge",            MoveKind::RECHARGE},
        {"razor-wind",          MoveKind::TWO_TURN},
        {"skull-bash",          MoveKind::TWO_TURN},
        {"sky-attack",          MoveKind::TWO_TURN},
        {"solar-beam",          MoveKind::TWO_TURN},
        {"semi-invulnerable",   MoveKind::TWO_TURN},
        //  Fixed or conditional damage, or not a reliable attack.
        {"beat-up",             MoveKind::AVOID},
        {"bide",                MoveKind::AVOID},
        {"counter",             MoveKind::AVOID},
        {"mirror-coat",         MoveKind::AVOID},
        {"dragon-rage",         MoveKind::AVOID},
        {"sonicboom",           MoveKind::AVOID},
        {"level-damage",        MoveKind::AVOID},
        {"psywave",             MoveKind::AVOID},
        {"super-fang",          MoveKind::AVOID},
        {"endeavor",            MoveKind::AVOID},
        {"ohko",                MoveKind::AVOID},
        {"explosion",           MoveKind::AVOID},
        {"dream-eater",         MoveKind::AVOID},
        {"fake-out",            MoveKind::AVOID},
        {"false-swipe",         MoveKind::AVOID},
        {"focus-punch",         MoveKind::AVOID},
        {"future-sight",        MoveKind::AVOID},
        {"present",             MoveKind::AVOID},
        {"snore",               MoveKind::AVOID},
        {"spit-up",             MoveKind::AVOID},
    };
    auto iter = KINDS.find(effect);
    return iter == KINDS.end() ? MoveKind::NORMAL : iter->second;
}

//  The decomp gives moves whose power varies a power of 1. A typical value
//  for each, to compare them with other moves.
uint8_t typical_power(const std::string& effect, uint8_t power){
    static const std::map<std::string, uint8_t> POWERS{
        {"low-kick",        60},    //  By the target's weight.
        {"flail",           40},    //  Flail, Reversal: by the user's HP.
        {"return",          70},    //  By friendship.
        {"frustration",     70},
        {"magnitude",       70},    //  Random.
        {"hidden-power",    50},    //  By IVs.
    };
    auto iter = POWERS.find(effect);
    return iter == POWERS.end() ? power : iter->second;
}

uint8_t immunity_of_ability(const std::string& ability){
    static const std::map<std::string, uint8_t> IMMUNITIES{
        {"levitate",        IMMUNE_GROUND},
        {"flash-fire",      IMMUNE_FIRE},
        {"water-absorb",    IMMUNE_WATER},
        {"volt-absorb",     IMMUNE_ELECTRIC},
        {"wonder-guard",    IMMUNE_WONDER_GUARD},
    };
    auto iter = IMMUNITIES.find(ability);
    return iter == IMMUNITIES.end() ? 0 : iter->second;
}

//  BaseStats.json writes some slugs differently (e.g. "farfetch-d"), so
//  species are matched without hyphens.
std::string without_hyphens(const std::string& slug){
    std::string ret;
    for (char ch : slug){
        if (ch != '-'){
            ret += ch;
        }
    }
    return ret;
}

PokemonType read_type(const std::string& slug, const std::string& path){
    PokemonType type = Pokemon::POKEMON_TYPE_SLUGS().get_enum(slug, PokemonType::FAIRY);
    if (type == PokemonType::FAIRY){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Not a Gen 3 type: " + slug, path);
    }
    return type;
}

uint8_t read_byte(const JsonObject& obj, const std::string& key, const std::string& path){
    int64_t value = obj.get_integer_throw(key, path);
    if (value < 0 || value > 255){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Value out of range: " + key, path);
    }
    return (uint8_t)value;
}

}


double type_effectiveness(PokemonType attack, PokemonType defend){
    for (const TypeMatchup& item : TYPE_CHART){
        if (item.attack == attack && item.defend == defend){
            return item.multiplier;
        }
    }
    return 1.0;
}


const BattleDatabase& BattleDatabase::instance(){
    static BattleDatabase database;
    return database;
}

BattleDatabase::BattleDatabase(){
    {
        std::string path = RESOURCE_PATH() + "PokemonFRLG/BattleMoves.json";
        JsonValue json = load_json_file(path);
        const JsonObject& root = json.to_object_throw(path);
        for (const auto& item : root){
            const JsonObject& obj = item.second.to_object_throw(path);
            const std::string& effect = obj.get_string_throw("effect", path);
            BattleMove move;
            move.slug = item.first;
            move.type = read_type(obj.get_string_throw("type", path), path);
            move.power = typical_power(effect, read_byte(obj, "power", path));
            move.accuracy = read_byte(obj, "accuracy", path);
            move.pp = read_byte(obj, "pp", path);
            move.kind = kind_of_effect(effect);
            move.type_varies = effect == "hidden-power";
            m_moves.emplace(move.slug, std::move(move));
        }
    }

    //  Base stats: HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
    std::map<std::string, std::vector<uint8_t>> base_stats;
    {
        std::string path = RESOURCE_PATH() + "PokemonFRLG/BaseStats.json";
        JsonValue json = load_json_file(path);
        const JsonObject& root = json.to_object_throw(path);
        for (const auto& item : root){
            const JsonArray& array = item.second.to_object_throw(path).get_array_throw("baseStats", path);
            std::vector<uint8_t> stats;
            for (const JsonValue& stat : array){
                stats.emplace_back((uint8_t)stat.to_integer_throw(path));
            }
            if (stats.size() != 6){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid length for base stats.", path);
            }
            base_stats.emplace(without_hyphens(item.first), std::move(stats));
        }
    }

    {
        std::string path = RESOURCE_PATH() + "PokemonFRLG/BattleSpecies.json";
        JsonValue json = load_json_file(path);
        const JsonObject& root = json.to_object_throw(path);
        for (const auto& item : root){
            const JsonObject& obj = item.second.to_object_throw(path);
            auto stats = base_stats.find(without_hyphens(item.first));
            if (stats == base_stats.end()){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "No base stats for: " + item.first, path);
            }

            BattleSpecies species;
            species.slug = item.first;
            const JsonArray& types = obj.get_array_throw("types", path);
            if (types.size() != 2){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Expected two types for: " + item.first, path);
            }
            species.type1 = read_type(types[0].to_string_throw(path), path);
            species.type2 = read_type(types[1].to_string_throw(path), path);
            species.base_attack = stats->second[1];
            species.base_defense = stats->second[2];
            species.base_sp_attack = stats->second[3];
            species.base_sp_defense = stats->second[4];
            for (const JsonValue& ability : obj.get_array_throw("abilities", path)){
                species.immunities |= immunity_of_ability(ability.to_string_throw(path));
            }
            m_species.emplace(species.slug, std::move(species));
        }
    }
}

const BattleMove* BattleDatabase::move(const std::string& slug) const{
    auto iter = m_moves.find(slug);
    return iter == m_moves.end() ? nullptr : &iter->second;
}
const BattleSpecies* BattleDatabase::species(const std::string& slug) const{
    auto iter = m_species.find(slug);
    return iter == m_species.end() ? nullptr : &iter->second;
}



}
}
}
