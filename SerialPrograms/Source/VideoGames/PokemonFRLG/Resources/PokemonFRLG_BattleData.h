/*  Battle Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Gen 3 move and species data for choosing moves in battle, loaded from
 *  the Packages repo:
 *    - PokemonFRLG/BattleMoves.json:   type, power, accuracy, PP and effect
 *                                      of each move.
 *    - PokemonFRLG/BattleSpecies.json: types and abilities of each species.
 *    - PokemonFRLG/BaseStats.json:     base stats of each species.
 *  (All from the pokefirered decomp.) Plus the Gen 3 type chart.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleData_H
#define PokemonAutomation_PokemonFRLG_BattleData_H

#include <stdint.h>
#include <map>
#include <string>
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


//  In Gen 3 a move's type decides whether it uses Attack or Sp. Atk.
bool is_special_type(Pokemon::PokemonType type);

//  The Gen 3 type chart: the damage multiplier of an attack of one type on a
//  Pokemon of another (0, 0.5, 1 or 2). NONE is the "???" type of Curse.
double type_effectiveness(Pokemon::PokemonType attack, Pokemon::PokemonType defend);


//  How a move's power should be counted when comparing moves.
enum class MoveKind : uint8_t{
    NORMAL,
    MULTI_HIT,      //  2 to 5 hits, about 3 on average.
    DOUBLE_HIT,     //  Always 2 hits.
    TRIPLE_KICK,    //  Up to 3 hits of rising power.
    RECHARGE,       //  Hyper Beam: loses the next turn.
    TWO_TURN,       //  Charges or goes semi-invulnerable for a turn first.
    AVOID,          //  Not worth choosing: fixed or conditional damage, OHKO,
                    //  self-KO, False Swipe (can't finish a Pokemon), etc.
};

struct BattleMove{
    std::string slug;
    Pokemon::PokemonType type = Pokemon::PokemonType::NONE;
    //  0 for status moves. For moves whose power varies (Return, Low Kick,
    //  Magnitude, ...), a typical value.
    uint8_t power = 0;
    uint8_t accuracy = 0;           //  0 for moves that never miss.
    uint8_t pp = 0;
    MoveKind kind = MoveKind::NORMAL;
    bool type_varies = false;       //  Hidden Power: the type depends on IVs.
};

//  Abilities that make a species take no damage from some moves.
enum SpeciesImmunity : uint8_t{
    IMMUNE_GROUND       = 1 << 0,   //  Levitate
    IMMUNE_FIRE         = 1 << 1,   //  Flash Fire
    IMMUNE_WATER        = 1 << 2,   //  Water Absorb
    IMMUNE_ELECTRIC     = 1 << 3,   //  Volt Absorb
    IMMUNE_WONDER_GUARD = 1 << 4,   //  Only super effective moves hit.
};

struct BattleSpecies{
    std::string slug;
    Pokemon::PokemonType type1 = Pokemon::PokemonType::NONE;
    Pokemon::PokemonType type2 = Pokemon::PokemonType::NONE;   //  Same as type1 if single-typed.
    uint8_t base_attack = 0;
    uint8_t base_defense = 0;
    uint8_t base_sp_attack = 0;
    uint8_t base_sp_defense = 0;
    //  SpeciesImmunity flags. If any of the species' possible abilities gives
    //  an immunity, the species is treated as having it.
    uint8_t immunities = 0;
};


class BattleDatabase{
public:
    static const BattleDatabase& instance();

    //  By slug, e.g. "rock-slide" or "machamp". nullptr if unknown.
    const BattleMove* move(const std::string& slug) const;
    const BattleSpecies* species(const std::string& slug) const;

    const std::map<std::string, BattleMove>& moves() const{
        return m_moves;
    }

private:
    BattleDatabase();

    std::map<std::string, BattleMove> m_moves;
    std::map<std::string, BattleSpecies> m_species;
};



}
}
}
#endif
