/*  Pokemon Sword/Shield Type Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "PokemonSwSh_TypeMatchup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::map<PokemonType, std::map<PokemonType, double>> TYPE_EFFECTIVENESS{
    {PokemonType::NORMAL, {
        {PokemonType::ROCK, 0.5}, {PokemonType::STEEL, 0.5},
        {PokemonType::GHOST, 0.0},
    }},
    {PokemonType::FIRE, {
        {PokemonType::BUG, 2.0}, {PokemonType::STEEL, 2.0}, {PokemonType::GRASS, 2.0}, {PokemonType::ICE, 2.0},
        {PokemonType::ROCK, 0.5}, {PokemonType::FIRE, 0.5}, {PokemonType::WATER, 0.5}, {PokemonType::DRAGON, 0.5},
    }},
    {PokemonType::FIGHTING, {
        {PokemonType::NORMAL, 2.0}, {PokemonType::ROCK, 2.0}, {PokemonType::STEEL, 2.0}, {PokemonType::ICE, 2.0}, {PokemonType::DARK, 2.0},
        {PokemonType::FLYING, 0.5}, {PokemonType::POISON, 0.5}, {PokemonType::BUG, 0.5}, {PokemonType::PSYCHIC, 0.5}, {PokemonType::FAIRY, 0.5},
        {PokemonType::GHOST, 0.0},
    }},
    {PokemonType::WATER, {
        {PokemonType::GROUND, 2.0}, {PokemonType::ROCK, 2.0}, {PokemonType::FIRE, 2.0},
        {PokemonType::WATER, 0.5}, {PokemonType::GRASS, 0.5}, {PokemonType::DRAGON, 0.5},
    }},
    {PokemonType::FLYING, {
        {PokemonType::FIGHTING, 2.0}, {PokemonType::BUG, 2.0}, {PokemonType::GRASS, 2.0},
        {PokemonType::ROCK, 0.5}, {PokemonType::STEEL, 0.5}, {PokemonType::ELECTRIC, 0.5},
    }},
    {PokemonType::GRASS, {
        {PokemonType::GROUND, 2.0}, {PokemonType::ROCK, 2.0}, {PokemonType::WATER, 2.0},
        {PokemonType::FLYING, 0.5}, {PokemonType::POISON, 0.5}, {PokemonType::BUG, 0.5}, {PokemonType::STEEL, 0.5}, {PokemonType::FIRE, 0.5}, {PokemonType::GRASS, 0.5}, {PokemonType::DRAGON, 0.5},
    }},
    {PokemonType::POISON, {
        {PokemonType::GRASS, 2.0}, {PokemonType::FAIRY, 2.0},
        {PokemonType::POISON, 0.5}, {PokemonType::GROUND, 0.5}, {PokemonType::ROCK, 0.5}, {PokemonType::GHOST, 0.5},
        {PokemonType::STEEL, 0.0},
    }},
    {PokemonType::ELECTRIC, {
        {PokemonType::FLYING, 2.0}, {PokemonType::WATER, 2.0},
        {PokemonType::GRASS, 0.5}, {PokemonType::ELECTRIC, 0.5}, {PokemonType::DRAGON, 0.5},
        {PokemonType::GROUND, 0.0},
    }},
    {PokemonType::GROUND, {
        {PokemonType::POISON, 2.0}, {PokemonType::ROCK, 2.0}, {PokemonType::STEEL, 2.0}, {PokemonType::FIRE, 2.0}, {PokemonType::ELECTRIC, 2.0},
        {PokemonType::BUG, 0.5}, {PokemonType::GRASS, 0.5},
        {PokemonType::FLYING, 0.0},
    }},
    {PokemonType::PSYCHIC, {
        {PokemonType::FIGHTING, 2.0}, {PokemonType::POISON, 2.0},
        {PokemonType::STEEL, 0.5}, {PokemonType::PSYCHIC, 0.5},
        {PokemonType::DARK, 0.0},
    }},
    {PokemonType::ROCK, {
        {PokemonType::FLYING, 2.0}, {PokemonType::BUG, 2.0}, {PokemonType::FIRE, 2.0}, {PokemonType::ICE, 2.0},
        {PokemonType::FIGHTING, 0.5}, {PokemonType::GROUND, 0.5}, {PokemonType::STEEL, 0.5},
    }},
    {PokemonType::ICE, {
        {PokemonType::FLYING, 2.0}, {PokemonType::GROUND, 2.0}, {PokemonType::GRASS, 2.0}, {PokemonType::DRAGON, 2.0},
        {PokemonType::STEEL, 0.5}, {PokemonType::FIRE, 0.5}, {PokemonType::WATER, 0.5}, {PokemonType::ICE, 0.5},
    }},
    {PokemonType::BUG, {
        {PokemonType::GRASS, 2.0}, {PokemonType::PSYCHIC, 2.0}, {PokemonType::DARK, 2.0},
        {PokemonType::FIGHTING, 0.5}, {PokemonType::FLYING, 0.5}, {PokemonType::POISON, 0.5}, {PokemonType::GHOST, 0.5}, {PokemonType::STEEL, 0.5}, {PokemonType::FIRE, 0.5}, {PokemonType::FAIRY, 0.5},
    }},
    {PokemonType::DRAGON, {
        {PokemonType::DRAGON, 2.0},
        {PokemonType::STEEL, 0.5},
        {PokemonType::FAIRY, 0.0},
    }},
    {PokemonType::GHOST, {
        {PokemonType::GHOST, 2.0}, {PokemonType::PSYCHIC, 2.0},
        {PokemonType::DARK, 0.5},
        {PokemonType::NORMAL, 0.0},
    }},
    {PokemonType::DARK, {
        {PokemonType::GHOST, 2.0}, {PokemonType::PSYCHIC, 2.0},
        {PokemonType::FIGHTING, 0.5}, {PokemonType::DARK, 0.5}, {PokemonType::FAIRY, 0.5},
    }},
    {PokemonType::STEEL, {
        {PokemonType::ICE, 2.0}, {PokemonType::ROCK, 2.0}, {PokemonType::FAIRY, 2.0},
        {PokemonType::STEEL, 0.5}, {PokemonType::FIRE, 0.5}, {PokemonType::WATER, 0.5}, {PokemonType::ELECTRIC, 0.5},
    }},
    {PokemonType::FAIRY, {
        {PokemonType::FIGHTING, 2.0}, {PokemonType::DRAGON, 2.0}, {PokemonType::DARK, 2.0},
        {PokemonType::POISON, 0.5}, {PokemonType::STEEL, 0.5}, {PokemonType::FIRE, 0.5},
    }},
};

double type_multiplier(PokemonType attack_type, PokemonType defend_type0, PokemonType defend_type1){
    auto iter = TYPE_EFFECTIVENESS.find(attack_type);
    if (iter == TYPE_EFFECTIVENESS.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Type Enum: " + std::to_string((int)attack_type));
    }
    double multiplier = 1.0;
    const std::map<PokemonType, double>& overrides = iter->second;
    auto iter0 = overrides.find(defend_type0);
    if (iter0 != overrides.end()) multiplier *= iter0->second;
    auto iter1 = overrides.find(defend_type1);
    if (iter1 != overrides.end()) multiplier *= iter1->second;
    return multiplier;
}




}
}
}
