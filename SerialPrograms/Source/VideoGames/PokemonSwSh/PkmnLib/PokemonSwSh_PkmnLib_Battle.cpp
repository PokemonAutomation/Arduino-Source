/*  PkmnLib Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include "PokemonSwSh_PkmnLib_Battle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


void calc_damage_range(
    uint8_t power,
    uint8_t level, uint16_t attack, uint16_t defense, double modifier,
    uint16_t& lower_bound, uint16_t& upper_bound
){
    // this calculates the damage ranges for an attack
    // every division is integer division except for the attack and defense ratio so no need for casting
    double attack_defense = (double)attack / (double)defense;

    int base_damage = (int)((((2 * (int)level) / 5 + 2) * power * (attack_defense)) / 50 + 2);

    // so, the modifier is usually just one, but we can calculate it before hand based on targets, weather, etc.
    int bottom_base = base_damage * 85 / 100;

    // BUG: there is currently a bug when calculating damage
    // due to all of the integer math that happens, the "modifier"
    // is actually often calculated one step at a time
    // so, the upper-end might be a little high in some cases

    // update the output "inputs"
    lower_bound = (uint16_t)(bottom_base * modifier);
    upper_bound = (uint16_t)(base_damage * modifier);
    // done
}

double ability_damage_multiplier(const Pokemon& attacker, const Pokemon& defender, const Move& move){

    // final multiplier that we're going to modify
    double multiplier = 1.0;

    const std::string& attacker_ability = attacker.ability();
    const std::string& defender_ability = defender.ability();
//    const std::string& move_name = attack_move.name();
    Type move_type = move.type();

    do{
        if (attacker_ability == "mold-breaker") break;
        if (attacker_ability == "turboblaze") break;
        if (attacker_ability == "teravolt") break;
        if (attacker_ability == "tinted-lens"){
            if (damage_multiplier(move_type, defender.type1(), defender.type2()) < 1.0){
                multiplier *= 2.0;
            }
            break;
        }

        switch (move_type){
        case Type::ground:
            if (defender_ability == "levitate"){
                if (move == "thousand-arrows"){
                    multiplier = 1.0;
                }else{
                    multiplier = 0.0;
                }
            }
            break;

        case Type::water:
            if (defender_ability == "water-absorb" ||
                defender_ability == "storm-drain" ||
                defender_ability == "dry-skin"
            ){
//                multiplier = -1.0;
                multiplier = 0.0;
            }
            break;

        case Type::fire:
            if (defender_ability == "flash-fire"){
//                multiplier = -1.0;
                multiplier = 0.0;
            }else if (defender_ability == "fluffy" || defender_ability == "dry-skin"){
                multiplier = 2.0;
            }else if (defender_ability == "thick-fat" || defender_ability == "heatproof"){
                multiplier = 0.5;
            }
            break;

        case Type::grass:
            if (defender_ability == "sap-sipper"){
//                multiplier = -1.0;
                multiplier = 0.0;
            }
            break;

        case Type::electric:
            if (defender_ability == "lightning-rod" ||
                defender_ability == "motor-drive" ||
                defender_ability == "volt-absorb"
            ){
//                multiplier = -1.0;
                multiplier = 0.0;
            }
            break;

        case Type::ice:
            if (defender_ability == "thick-fat"){
                multiplier = 0.5;
            }
            break;

        default:;
        }
    }while (false);

    if (attacker_ability == "iron-fist"){
        static std::set<std::string> MOVES{
            verify_move_slug("bullet-punch"),
            verify_move_slug("comet-punch"),
            verify_move_slug("dizzy-punch"),
            verify_move_slug("double-iron-bash"),
            verify_move_slug("drain-punch"),
            verify_move_slug("dynamic-punch"),
            verify_move_slug("fire-punch"),
            verify_move_slug("focus-punch"),
            verify_move_slug("hammer-arm"),
            verify_move_slug("ice-hammer"),
            verify_move_slug("ice-punch"),
            verify_move_slug("mach-punch"),
            verify_move_slug("mega-punch"),
            verify_move_slug("meteor-mash"),
            verify_move_slug("plasma-fists"),
            verify_move_slug("power-up-punch"),
            verify_move_slug("shadow-punch"),
            verify_move_slug("sky-uppercut"),
            verify_move_slug("surging-strikes"),
            verify_move_slug("thunder-punch"),
            verify_move_slug("wicked-blow"),
        };
        if (MOVES.find(move.name()) != MOVES.end()){
            multiplier *= 1.2;
        }
    }else if (attacker_ability == "strong-jaw"){
        static std::set<std::string> MOVES{
            verify_move_slug("bite"),
            verify_move_slug("crunch"),
            verify_move_slug("fire-fang"),
            verify_move_slug("fishious-rend"),
            verify_move_slug("hyper-fang"),
            verify_move_slug("ice-fang"),
            verify_move_slug("jaw-lock"),
            verify_move_slug("poison-fang"),
            verify_move_slug("psychic-fangs"),
            verify_move_slug("thunder-fang"),
        };
        if (MOVES.find(move.name()) != MOVES.end()){
            multiplier *= 1.5;
        }
    }else if (attacker_ability == "adaptability"){
        if (move_type == attacker.type1() || move_type == attacker.type2()){
            multiplier *= (4.0 / 3.0);
        }
    }

    return multiplier;
}

double weather_damage_multiplier(Move& move, const Field& field){

    double modifier = 1.0;

//    const std::string& attacker_ability = attacker.ability();
//    const std::string& defender_ability = defender.ability();
//    const std::string& move_name = move.name();
    Type move_type = move.type();

    switch (field.weather()){
    case Weather::CLEAR:
        if (move == "weather-ball"){
            move.set_type(Type::normal);
        }
        break;
    case Weather::SUN:
        if (move == "solar-beam"){
            modifier *= 2.0;
        }else if (move == "thunder" || move == "hurricane"){
            modifier *= (0.5 / 0.7);
        }else if (move == "weather-ball"){
            move.set_type(Type::fire);
            modifier *= 2.0;
        }

        if (move_type == Type::fire){
            modifier *= 1.5;
        }else if (move_type == Type::water){
            modifier *= 0.5;
        }
        break;
    case Weather::RAIN:
        if (move == "solar-beam"){
            modifier *= 0.5;
        }else if (move == "thunder" || move == "hurricane"){
            modifier *= (1.0 / 0.7);
        }else if (move == "weather-ball"){
            move.set_type(Type::water);
            modifier *= 2.0;
        }

        if (move_type == Type::fire){
            modifier *= 0.5;
        }else if (move_type == Type::water){
            modifier *= 1.5;
        }
        break;
    case Weather::HAIL:
        if (move == "solar-beam"){
            modifier *= 0.5;
        }else if (move == "blizzard"){
            modifier *= (1.0 / 0.7);
        }else if (move == "weather-ball"){
            modifier *= 2.0;
            move.set_type(Type::ice);
        }
        break;
    case Weather::SANDSTORM:
        if (move == "solar-beam"){
            modifier *= 0.5;
        }else if (move == "weather-ball"){
            modifier *= 2.0;
            move.set_type(Type::rock);
        }
        break;
    }

    return modifier;
}

double terrain_damage_multiplier(const Pokemon& attacker, const Pokemon& defender, Move& move, const Field& field){
    double modifier = 1.0;

    // if there's no terrain, just exit
    if (field.is_none_terrain()){
        return modifier;
    }

//    const std::string& attacker_ability = attacker.ability();
//    const std::string& defender_ability = defender.ability();
//    const std::string& move_name = move.name();
    Type move_type = move.type();

    bool is_flying = (attacker.type1() == Type::flying) || (attacker.type2() == Type::flying);
    bool is_levitate = attacker.ability() == "levitate";

    if (!is_flying || !is_levitate)
    {
        // TODO: investigate the modifier based on terrain pulse
        // bulbapedia says that the power *doubles* to 100 *and* gets powered up from the terrain
        switch (field.terrain()){
        case Terrain::NONE:
            break;
        case Terrain::ELECTRIC:
            if (move == "terrain-pulse"){
                move.set_type(Type::electric);
                modifier *= 1.5;
            }
            if (move_type == Type::electric){
                modifier *= 1.3;
            }
            break;
        case Terrain::GRASSY:
            if (move == "terrain-pulse"){
                move.set_type(Type::grass);
                modifier *= 1.5;
            }
            if (move_type == Type::grass){
                modifier *= 1.3;
            }
            break;
        case Terrain::PSYCHIC:
            if (move == "terrain-pulse"){
                move.set_type(Type::psychic);
                modifier *= 1.5;
            }else if (move == "expanding-force"){
                modifier *= 1.5;
            }
            if (move_type == Type::psychic){
                modifier *= 1.3;
            }
            break;
        case Terrain::MISTY:
            if (move == "terrain-pulse"){
                move.set_type(Type::fairy);
//                modifier *= 1.3;
            }
            if (move_type == Type::dragon){
                modifier *= 0.5;
            }
            break;
        }
    }

    // rising voltage applies if the defender is affected by the terrain
    is_flying = (defender.type1() == Type::flying) || (defender.type2() == Type::flying);
    is_levitate = defender.ability() == "levitate";

    if (!is_flying || !is_levitate){
        if (field.is_electric()){
            if (move == "rising-voltage"){
                modifier *= 2.0;
            }
        }
    }

    return modifier;
}

double damage_score(
    const Pokemon& attacker, const Pokemon& defender,
    size_t moveIdx, const Field& field, bool multipleTargets
){

    // get the right attacker move
    Move move = attacker.is_dynamax() ? attacker.max_move(moveIdx) : attacker.move(moveIdx);

    // NOTE: starting the multiplier that we're using at 1.0 because the damage calculation function
    // includes the "ranges" for high and low damage, so we can use that to calculate an average
    double avgMultiplier = move.correction_factor();
    // NOTE: the original function used to call accuracy here, it's now included at the end
    // avgMultiplier *= move.getAccuracy();

    // then if we're dealing with multiple targets and it's a spread
    if (multipleTargets && move.is_spread()){
        avgMultiplier *= 0.75;
    }

    // ignore crits for now
    // TODO: any high-crit chance moves could be incorporated

    // get the weather damage mod
    avgMultiplier *= weather_damage_multiplier(move, field);
    // get the terrain damage mod
    avgMultiplier *= terrain_damage_multiplier(attacker, defender, move, field);

    // calculations for refrigerate, aerilate, galvanize, pixilate, and normalize done before stab
    if (move.type() == Type::normal){
        if (attacker.ability() == "refrigerate"){
            move.set_type(Type::ice);
            avgMultiplier *= 1.2;
        }else if (attacker.ability() == "aerilate"){
            move.set_type(Type::flying);
            avgMultiplier *= 1.2;
        }else if (attacker.ability() == "galvanize"){
            move.set_type(Type::electric);
            avgMultiplier *= 1.2;
        }else if (attacker.ability() == "pixilate"){
            move.set_type(Type::fairy);
            avgMultiplier *= 1.2;
        }
    }else{
        if (attacker.ability() == "normalize"){
            move.set_type(Type::normal);
            avgMultiplier *= 1.2;
        }
    }

    // now we determine stab
    if (attacker.is_stab(move.type())){
        avgMultiplier *= 1.5;
    }

    // and now we deal with type effectiveness
    if (!(move == "thousand-arrows" && defender.has_type(Type::flying))){
        avgMultiplier *= damage_multiplier(move.type(), defender.type1(), defender.type2());
    }

    // apply status effects
    if (move.category() == MoveCategory::PHYSICAL && attacker.non_volatile_status_effect() == NonVolatileStatusEffects::BURN){
        avgMultiplier *= 0.5;
    }

    // apply modifiers from ability
    avgMultiplier *= ability_damage_multiplier(attacker, defender, move);

    // apply boosts from aura
    if (move.type() == Type::fairy && (attacker.ability() == "fairy-aura" || defender.ability() == "fairy-aura")){
        avgMultiplier *= 1.33;
    }

    if (move.type() == Type::dark && (attacker.ability() == "dark-aura" || defender.ability() == "dark-aura")){
        avgMultiplier *= 1.33;
    }

    // attacker and defender stats time
    // declare the attack and defense stats
    uint16_t attackUse, defenseUse;

    if (move.category() == MoveCategory::PHYSICAL){
        if (move == "body-press"){
            attackUse = attacker.defense();
        }else if (move == "foul-play"){
            attackUse = defender.attack();
        }else{
            attackUse = attacker.attack();
        }
        defenseUse = defender.defense();
    }else{
        attackUse = attacker.special_attack();
        if (move == "psystrike" || move == "psyshock"){
            defenseUse = defender.defense();
        }else{
            defenseUse = defender.special_defense();
        }
    }

    // then calculate the damage
    uint16_t damageLow, damageHigh;
    // NOTE: the calcDamageRanges function will give the max damage and the min damage
    // no need to include the 0.925 modifier above!
    calc_damage_range(
        move.base_power(), attacker.level(), attackUse, defenseUse, avgMultiplier,
        damageLow, damageHigh
    );

    // so get the average between the two multiplied by accuracy
    double damageScore = (damageLow + damageHigh) * move.accuracy() / 2;

//    // return the move type back to its original value
//    move.reset_move_type();

    return damageScore;
}





















}
}
}
}
