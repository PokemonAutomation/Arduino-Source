/*  Battle Move Choice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <utility>
#include <vector>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_BattleMoveReader.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG_BattleMenuNavigation.h"
#include "PokemonFRLG_BattleMoveChoice.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using Pokemon::PokemonType;


double score_move(
    const BattleMove& move,
    const BattleSpecies* attacker,
    const BattleSpecies* defender
){
    if (move.kind == MoveKind::AVOID || move.power == 0 || move.type == PokemonType::NONE){
        return 0;
    }

    double score = move.power;

    switch (move.kind){
    case MoveKind::MULTI_HIT:
        score *= 3.0;
        break;
    case MoveKind::DOUBLE_HIT:
        score *= 2.0;
        break;
    case MoveKind::TRIPLE_KICK:
        score *= 4.0;   //  10 + 20 + 30 if every kick hits; less if one misses.
        break;
    case MoveKind::RECHARGE:
        score *= 0.6;
        break;
    case MoveKind::TWO_TURN:
        score *= 0.5;
        break;
    default:
        break;
    }

    if (move.accuracy != 0){
        score *= move.accuracy / 100.0;
    }

    //  Hidden Power's type depends on the Pokemon's IVs: no STAB or type
    //  matchups for it.
    if (attacker != nullptr && !move.type_varies){
        if (move.type == attacker->type1 || move.type == attacker->type2){
            score *= 1.5;
        }
    }

    //  Physical or special, by type. Base stats stand in for the real stats.
    const bool special = is_special_type(move.type);
    if (attacker != nullptr){
        score *= special ? attacker->base_sp_attack : attacker->base_attack;
    }
    if (defender != nullptr){
        score /= std::max<uint8_t>(1, special ? defender->base_sp_defense : defender->base_defense);
    }

    if (defender != nullptr && !move.type_varies){
        double multiplier = type_effectiveness(move.type, defender->type1);
        if (defender->type2 != defender->type1){
            multiplier *= type_effectiveness(move.type, defender->type2);
        }

        uint8_t immunity = 0;
        switch (move.type){
        case PokemonType::GROUND:
            immunity = IMMUNE_GROUND;
            break;
        case PokemonType::FIRE:
            immunity = IMMUNE_FIRE;
            break;
        case PokemonType::WATER:
            immunity = IMMUNE_WATER;
            break;
        case PokemonType::ELECTRIC:
            immunity = IMMUNE_ELECTRIC;
            break;
        default:
            break;
        }
        if (defender->immunities & immunity){
            multiplier = 0;
        }
        if ((defender->immunities & IMMUNE_WONDER_GUARD) && multiplier <= 1.0){
            multiplier = 0;
        }
        score *= multiplier;
    }

    return score;
}

std::array<int, 4> rank_moves(
    const MoveSlots& moves,
    const BattleSpecies* attacker,
    const BattleSpecies* defender,
    size_t& usable
){
    const BattleDatabase& database = BattleDatabase::instance();
    std::array<std::pair<double, int>, 4> scored;
    for (int i = 0; i < 4; i++){
        const BattleMove* move = moves[(size_t)i].empty() ? nullptr : database.move(moves[(size_t)i]);
        scored[(size_t)i] = {move == nullptr ? 0.0 : score_move(*move, attacker, defender), i};
    }
    std::stable_sort(
        scored.begin(), scored.end(),
        [](const std::pair<double, int>& a, const std::pair<double, int>& b){
            return a.first > b.first;
        }
    );

    std::array<int, 4> ret{-1, -1, -1, -1};
    usable = 0;
    for (const auto& item : scored){
        if (item.first > 0){
            ret[usable++] = item.second;
        }
    }
    return ret;
}


void MoveChoiceState::new_battle(){
    grudge_active = false;
    grudge_stalls = 0;
    unusable_slot = -1;
    unusable_turns = 0;
}

void MoveChoiceState::forget_own_pokemon(){
    moves_read = false;
    species_read = false;
}


namespace{

//  Add every non-empty slot that isn't listed yet (and isn't `skip`) to the
//  end of `candidates`.
void add_other_moves(std::vector<int>& candidates, const MoveSlots& moves, int skip){
    for (int slot = 0; slot < 4; slot++){
        bool listed = slot == skip ||
            std::find(candidates.begin(), candidates.end(), slot) != candidates.end();
        if (!listed && !moves[(size_t)slot].empty()){
            candidates.push_back(slot);
        }
    }
}

}


bool use_best_move(
    ConsoleHandle& console, ProControllerContext& context,
    MoveChoiceState& state
){
    BattleMoveReader move_reader(state.language);
    VideoOverlaySet overlays(console.overlay());
    move_reader.make_overlays(overlays);
    const BattleDatabase& database = BattleDatabase::instance();

    //  Our species, in single battles. (Unknown in double battles, where the
    //  HP boxes are laid out differently: STAB is then ignored.)
    if (!state.double_battle && !state.species_read){
        VideoSnapshot screen = console.video().snapshot();
        state.own_species = move_reader.read_own_species(console.logger(), screen);
        state.species_read = true;
        if (database.species(state.own_species) == nullptr){
            console.log("Battling Pokemon not recognized (nickname?). Ignoring its types and stats when choosing moves.", COLOR_ORANGE);
        }else{
            console.log("Battling Pokemon: " + state.own_species);
        }
    }
    const BattleSpecies* attacker = state.double_battle ? nullptr : database.species(state.own_species);

    //  Who are we fighting?
    std::string opponent;
    {
        WildEncounterReader reader(COLOR_RED);
        reader.make_overlays(overlays);
        VideoSnapshot screen = console.video().snapshot();
        opponent = reader.read_encounter(console.logger(), state.language, screen, state.opponent_species).name;
    }
    console.log("Opponent: " + (opponent.empty() ? std::string("(unread)") : opponent));

    //  Open the move list.
    if (!move_cursor_to_option(console, context, BattleMenuOption::FIGHT)){
        console.log("Could not confirm the cursor is on FIGHT. Pressing A anyway.", COLOR_RED);
    }
    pbf_press_button(context, BUTTON_A, 200ms, 500ms);
    context.wait_for_all_requests();

    if (state.double_battle || !state.moves_read){
        VideoSnapshot screen = console.video().snapshot();
        state.moves = move_reader.read_moves(console.logger(), screen);
        state.moves_read = !state.double_battle;
        std::string log = "Moves:";
        for (const std::string& move : state.moves){
            log += " [" + (move.empty() ? std::string("-") : move) + "]";
        }
        console.log(log);
    }
    const MoveSlots& moves = state.moves;

    size_t usable = 0;
    std::array<int, 4> ranked = rank_moves(moves, attacker, database.species(opponent), usable);

    //  An unusable move is skipped for a few turns, then tried again.
    if (state.unusable_turns > 0 && --state.unusable_turns == 0){
        state.unusable_slot = -1;
    }

    //  The damaging moves, best first, without an unusable one. If a damaging
    //  move is unusable, any other move (even one that does nothing to this
    //  opponent) keeps the battle going until it can be used again.
    std::vector<int> candidates;
    bool damaging_unusable = false;
    for (size_t i = 0; i < usable; i++){
        if (ranked[i] == state.unusable_slot){
            damaging_unusable = true;
        }else{
            candidates.push_back(ranked[i]);
        }
    }
    if (damaging_unusable){
        add_other_moves(candidates, moves, state.unusable_slot);
    }

    //  Grudge: if the opponent faints from the move used now, that move loses
    //  all its PP. Use the next best attacking move instead. With only one,
    //  use something else for a turn (Grudge ends when the opponent moves
    //  again). The Grudge message can scroll by unseen, so against opponents
    //  known to use it, the best move is always kept back if there is another.
    bool grudge_seen = state.grudge_active;
    state.grudge_active = false;
    if (grudge_seen || state.grudge_opponents.count(opponent) != 0){
        if (candidates.size() >= 2 && !damaging_unusable){
            console.log("Grudge: using " + moves[(size_t)candidates[1]] + " instead of " + moves[(size_t)candidates[0]] + ".");
            std::swap(candidates[0], candidates[1]);
        }else if (grudge_seen && candidates.size() == 1 && state.grudge_stalls < 2){
            std::vector<int> others;
            add_other_moves(others, moves, candidates[0]);
            others.erase(
                std::remove(others.begin(), others.end(), state.unusable_slot),
                others.end()
            );
            if (!others.empty()){
                state.grudge_stalls++;
                console.log("Grudge, and only one attacking move: using another move this turn.");
                others.push_back(candidates[0]);
                candidates = others;
            }
        }
    }

    //  (By index: a move found unusable below adds candidates.)
    for (size_t c = 0; c < candidates.size(); c++){
        int slot = candidates[c];
        const std::string& name = moves[(size_t)slot];
        if (!move_cursor_to_option(console, context, static_cast<BattleMoveOption>(slot))){
            console.log("Could not find the move cursor. Backing out to the battle menu.", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 200ms, 800ms);
            return true;
        }
        BattleOutOfPpWatcher out_of_pp(COLOR_RED);
        if (wait_until(console, context, 500ms, { out_of_pp }) == 0){
            console.log(name + " is out of PP.", COLOR_ORANGE);
            continue;
        }
        console.log("Using " + name + " (slot " + std::to_string(slot + 1) + ").");
        pbf_press_button(context, BUTTON_A, 200ms, 0ms);
        context.wait_for_all_requests();

        //  A move that can't be used right now (Disable, Taunt, Torment, ...)
        //  shows a message that waits for a button, then goes back to the
        //  move list. A move that is used starts the turn, whose first
        //  messages don't wait.
        AdvanceBattleDialogWatcher cant_use(COLOR_RED);
        if (wait_until(console, context, 1200ms, { cant_use }) == 0){
            console.log(name + " can't be used right now. Using another move for a few turns.", COLOR_ORANGE);
            state.unusable_slot = slot;
            state.unusable_turns = 3;
            pbf_press_button(context, BUTTON_B, 200ms, 1000ms);
            context.wait_for_all_requests();
            if (!damaging_unusable){
                damaging_unusable = true;
                add_other_moves(candidates, moves, slot);
            }
            continue;
        }

        //  In a double battle, a move aimed at one Pokemon then asks for its
        //  target: the move list stays up and the target blinks. A confirms
        //  the one picked. (Moves that hit both foes go on right away.)
        if (state.double_battle){
            BattleSelectionArrowWatcher choosing_target(COLOR_RED, &console.overlay(), BATTLE_MOVE_ARROW_BOX);
            if (wait_until(console, context, 500ms, { choosing_target }) == 0){
                console.log("Confirming the target.");
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                context.wait_for_all_requests();
            }
        }

        //  A move that can't be chosen without a message stays on the list.
        BattleSelectionArrowWatcher still_on_list(COLOR_RED, &console.overlay(), BATTLE_MOVE_ARROW_BOX);
        if (wait_until(console, context, 1500ms, { still_on_list }) == 0){
            console.log(name + " can't be used right now. Trying the next move.", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_B, 200ms, 600ms);
            context.wait_for_all_requests();
            continue;
        }
        return true;
    }
    pbf_press_button(context, BUTTON_B, 200ms, 600ms);
    return false;
}



}
}
}
