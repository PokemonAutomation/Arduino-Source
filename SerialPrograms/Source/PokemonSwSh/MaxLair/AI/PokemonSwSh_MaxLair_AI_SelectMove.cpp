/*  Max Lair AI Select Move
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_AI_Tools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



std::pair<uint8_t, bool> select_move_ai(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    using namespace papkmnlib;

    //  Build PkmnLib classes.
    Pokemon boss = get_pokemon(*state.opponent.begin());
    boss.set_hp_ratio(state.opponent_hp);

    std::unique_ptr<Pokemon> self;
    std::vector<std::unique_ptr<Pokemon>> teammates;

    for (size_t c = 0; c < 4; c++){
        const PlayerState& player = state.players[c];
        if (player.pokemon.empty()){
            continue;
        }

        std::unique_ptr<Pokemon> pokemon = convert_player_to_pkmnlib(player);
        pokemon->transform_from_ditto(boss);

        if (c == player_index){
            self = std::move(pokemon);
        }else{
            teammates.emplace_back(std::move(pokemon));
        }
    }

    //  TODO: Proper field detection.
    Field field;
    field.set_default_field(boss.name());

    std::vector<const Pokemon*> teammates_v;
    for (const std::unique_ptr<Pokemon>& item : teammates){
        teammates_v.emplace_back(item.get());
    }

    std::multimap<double, std::pair<uint8_t, bool>, std::greater<double>> rank;

    //  No dmax.
    if (state.players[player_index].dmax_turns_left <= 0){
        for (size_t c = 0; c < self->num_moves(); c++){
            if (self->pp(c) <= 0){
                continue;
            }
            if (state.players[player_index].move_blocked[c]){
                continue;
            }
            double score = calc_move_score(*self, boss, teammates_v, c, field);
            rank.emplace(
                score,
                std::pair<uint8_t, bool>{(uint8_t)c, false}
            );
        }
    }

    //  Dmax
    self->set_is_dynamax(true);
    if (state.players[player_index].dmax_turns_left > 0 || state.players[player_index].can_dmax){
        for (size_t c = 0; c < self->num_moves(); c++){
            if (self->pp(c) <= 0){
                continue;
            }
            if (state.players[player_index].move_blocked[c]){
                continue;
            }
            double score = calc_move_score(*self, boss, teammates_v, c, field);
            rank.emplace(
                score,
                std::pair<uint8_t, bool>{(uint8_t)c, true}
            );
        }
    }

    //  Print options and scores.
    std::string move_dump = "Move Score:\n";
    for (const auto& move : rank){
        uint8_t slot = move.second.first;
        move_dump += std::to_string(move.first) + " : ";
        move_dump += move.second.second
            ? self->max_move(slot).name()
            : self->move(slot).name();
        move_dump += "\n";
    }
    logger.log(move_dump);

    if (rank.empty()){
        logger.log("Unable to calculate moves. Picking a random move...", COLOR_RED);
        return {(uint8_t)random(0, 3), false};
    }
    if (rank.begin()->first < 0){
        logger.log("No viable moves found. Picking a random move...", COLOR_RED);
        return {(uint8_t)random(0, 3), false};
    }

    return rank.begin()->second;
}





//  Return 0 - 3 for move position. 2nd parameter is dmax.
std::pair<uint8_t, bool> select_move(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Selecting a move... State =>\n" + state.dump(),
        COLOR_PURPLE
    );

    if (state.opponent.empty()){
        logger.log("Don't know who the opponent is. Picking a random move...", COLOR_RED);
        return {(uint8_t)random(0, 3), false};
    }

    const PlayerState& player = state.players[player_index];
    const MaxLairMon* self = get_maxlair_mon_nothrow(player.pokemon);
    if (self == nullptr){
        logger.log("Don't know what you are. Picking a random move...", COLOR_RED);
        return {(uint8_t)random(0, 3), false};
    }

    std::pair<uint8_t, bool> result = select_move_ai(logger, state, player_index);
    if (player.move_blocked[result.first]){
        logger.log("AI picked an unusable move. Picking random move to avoid hanging...", COLOR_RED);
        return {(uint8_t)random(0, 3), false};
    }

    return result;
}



}
}
}
}
