/*  Battle Move Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Gen 3 move, species and type data (from the pokefirered decomp), and a
 *  scorer that picks the strongest move against an opponent.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleMoveData_H
#define PokemonAutomation_PokemonFRLG_BattleMoveData_H

#include <array>
#include <cstdint>
#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class BattleType : uint8_t{
    NORMAL,
    FIGHTING,
    FLYING,
    POISON,
    GROUND,
    ROCK,
    BUG,
    GHOST,
    STEEL,
    NONE,       //  "???" (Curse). Never deals typed damage.
    FIRE,
    WATER,
    GRASS,
    ELECTRIC,
    PSYCHIC,
    ICE,
    DRAGON,
    DARK,
};

//  In Gen 3 the move's type decides whether Attack or Sp. Atk is used.
bool is_special_type(BattleType type);

//  How a move's power should be read when scoring it.
enum class MoveKind : uint8_t{
    NORMAL,
    MULTI_HIT,      //  2-5 hits, about 3 on average.
    DOUBLE_HIT,     //  Always 2 hits.
    TRIPLE_KICK,    //  Up to 3 hits of rising power.
    RECHARGE,       //  Hyper Beam: loses the next turn.
    TWO_TURN,       //  Charges or goes semi-invulnerable for a turn first.
    AVOID,          //  Never chosen: fixed or conditional damage, OHKO, self-KO,
                    //  False Swipe (can't finish a Pokemon), etc.
};

struct MoveInfo{
    const char* slug;           //  OCR token, e.g. "rock-slide".
    const char* display_name;   //  As the game shows it, e.g. "ROCK SLIDE".
    BattleType type;
    uint8_t power;              //  0 for status moves.
    uint8_t accuracy;           //  0 for moves that never miss.
    MoveKind kind;
};

enum SpeciesImmunity : uint8_t{
    IMMUNE_GROUND       = 1 << 0,   //  Levitate
    IMMUNE_FIRE         = 1 << 1,   //  Flash Fire
    IMMUNE_WATER        = 1 << 2,   //  Water Absorb
    IMMUNE_ELECTRIC     = 1 << 3,   //  Volt Absorb
    IMMUNE_WONDER_GUARD = 1 << 4,   //  Only super effective moves hit.
};

struct SpeciesInfo{
    const char* slug;           //  Same slugs as Pokemon::PokemonNameReader.
    BattleType type1;
    BattleType type2;
    uint8_t base_attack;
    uint8_t base_defense;
    uint8_t base_sp_attack;
    uint8_t base_sp_defense;
    //  Abilities that can make a type deal no damage. If any of the species'
    //  possible abilities does that, the species is treated as immune.
    uint8_t immunities;
};

//  nullptr if unknown.
const MoveInfo* find_move(const std::string& slug);
const SpeciesInfo* find_species(const std::string& slug);

//  Every move slug, for building the OCR dictionary.
const MoveInfo* all_moves(size_t& count);

double type_effectiveness(BattleType attack, BattleType defend);


//  The four moves of the battling Pokemon, top-left, top-right, bottom-left,
//  bottom-right. Empty slugs are empty or unreadable slots.
using MoveSlots = std::array<std::string, 4>;

//  Relative damage of a move against the opponent. Only useful for comparing
//  the moves of one Pokemon against one opponent. 0 means "don't use".
//    - attacker: our species, or nullptr if unknown (then STAB and the
//      Attack/Sp. Atk balance are ignored).
//    - defender: the opponent, or nullptr if unknown (then type matchups and
//      its defenses are ignored).
double score_move(
    const MoveInfo& move,
    const SpeciesInfo* attacker,
    const SpeciesInfo* defender
);

//  Order the move slots from best to worst against the defender. Slots that
//  should never be used (unknown move, no damage, no effect) are left out.
std::array<int, 4> rank_moves(
    const MoveSlots& moves,
    const SpeciesInfo* attacker,
    const SpeciesInfo* defender,
    size_t& usable
);



}
}
}
#endif
