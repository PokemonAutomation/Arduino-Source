/*  Battle Move Choice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Pick the strongest move against the opponent and use it: by the moves'
 *  power, accuracy and type against the opponent's types, abilities and
 *  base stats. For programs that win trainer battles automatically.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleMoveChoice_H
#define PokemonAutomation_PokemonFRLG_BattleMoveChoice_H

#include <array>
#include <set>
#include <string>
#include "CommonFramework/Language.h"
#include "VideoGames/PokemonFRLG/Resources/PokemonFRLG_BattleData.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonFRLG{


//  The four moves of a Pokemon as move slugs, top-left, top-right,
//  bottom-left, bottom-right. Empty for an empty or unreadable slot.
using MoveSlots = std::array<std::string, 4>;


//  Relative damage of a move against the opponent, for comparing the moves
//  of one Pokemon against one opponent. 0 means "don't use".
//    - attacker: our species, or nullptr if unknown (then STAB and the
//      Attack / Sp. Atk balance are ignored).
//    - defender: the opponent, or nullptr if unknown (then type matchups,
//      immunities and its defenses are ignored).
double score_move(
    const BattleMove& move,
    const BattleSpecies* attacker,
    const BattleSpecies* defender
);

//  The move slots (0 to 3) worth using against the defender, best first:
//  the first `usable` entries. The rest are -1. (Unknown moves and moves
//  that do no damage or have no effect are left out.)
std::array<int, 4> rank_moves(
    const MoveSlots& moves,
    const BattleSpecies* attacker,
    const BattleSpecies* defender,
    size_t& usable
);


//  What use_best_move() keeps between turns. Call new_battle() at the start
//  of each battle.
struct MoveChoiceState{
    Language language = Language::English;

    //  Two Pokemon on each side. The menu then comes up for each of ours in
    //  turn, and their moves are read every time (the HP box readers only
    //  work in single battles).
    bool double_battle = false;

    //  Who the opponents can be (species slugs), to read them.
    std::set<std::string> opponent_species;

    //  Opponents that know Grudge: against them, the best move is kept back
    //  if there is another attacking move. (If an opponent faints from a
    //  move right after using Grudge, that move loses all its PP.)
    std::set<std::string> grudge_opponents;

    //  Set by the caller when it sees an opponent use Grudge: the next move
    //  is a different attacking move, or, with only one, another move (at
    //  most twice per battle).
    bool grudge_active = false;

    //  Single battles: our battling Pokemon's moves and species, read once
    //  and kept until forget_own_pokemon().
    bool moves_read = false;
    MoveSlots moves;
    bool species_read = false;
    std::string own_species;

    //  A move that couldn't be used (e.g. Disable lasts 2 to 5 turns). It is
    //  skipped for a few turns, then tried again.
    int unusable_slot = -1;
    int unusable_turns = 0;
    int grudge_stalls = 0;

    void new_battle();

    //  Read the battling Pokemon's moves and species again next turn: after
    //  a switch, a new move or an evolution.
    void forget_own_pokemon();
};


//  At the battle menu: open FIGHT, pick the best move against the opponent
//  and use it. Returns false (back at the battle menu) if none of the
//  damaging moves can be used, e.g. none has PP left.
bool use_best_move(
    ConsoleHandle& console, ProControllerContext& context,
    MoveChoiceState& state
);



}
}
}
#endif
