/*  Max Lair AI Swap with Professor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h"
#include "PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh_MaxLair_AI_Tools.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//  Professor offers to exchange a Pokemon.
bool should_swap_with_professor(
    Logger& logger,
    const GlobalState& state,
    size_t player_index
){
    logger.log(
        "Player " + std::to_string(player_index) +
        ": Deciding whether to swap with professor... State =>\n" + state.dump(),
        COLOR_PURPLE
    );

    using namespace papkmnlib;

    uint8_t lives = 4;


    std::vector<const Pokemon*> rental_candidates_on_path =
        get_rental_candidates_on_path_pkmnlib(state);

    std::vector<const Pokemon*> boss_candidates_on_path =
        get_boss_candidates(state);

    std::unique_ptr<Pokemon> current_team[4];
    current_team[0] = convert_player_to_pkmnlib(state.players[0]);
    current_team[1] = convert_player_to_pkmnlib(state.players[1]);
    current_team[2] = convert_player_to_pkmnlib(state.players[2]);
    current_team[3] = convert_player_to_pkmnlib(state.players[3]);


    //  Find the "average" rental against this boss.
    std::multimap<double, const Pokemon*> list;
    for (const Pokemon* boss : boss_candidates_on_path){
        for (const auto& rental : all_rental_pokemon()){
            if (state.seen.find(rental.first) != state.seen.end()){
                continue;
            }
            list.emplace(evaluate_matchup(rental.second, *boss, {}, lives), &rental.second);
        }
    }
    if (list.empty()){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Opponent candidate list is empty.");
    }
    size_t midpoint = list.size() / 2;
    for (size_t c = 0; c < midpoint; c++){
        list.erase(list.begin());
    }
    const Pokemon* average_rental = list.begin()->second;
    list.clear();



    std::multimap<double, int8_t, std::greater<double>> rank;
#if 0
    {
        const Pokemon* hypothetical_team[4];
        hypothetical_team[0] = current_team[0].get();
        hypothetical_team[1] = current_team[1].get();
        hypothetical_team[2] = current_team[2].get();
        hypothetical_team[3] = current_team[3].get();
        double score = evaluate_hypothetical_team(
            state,
            hypothetical_team,
            rental_candidates_on_path,
            boss_candidates_on_path
        );
        rank.emplace(score, -1);
    }
    for (int8_t c = 0; c < 4; c++){
        const Pokemon* hypothetical_team[4];
        hypothetical_team[0] = current_team[0].get();
        hypothetical_team[1] = current_team[1].get();
        hypothetical_team[2] = current_team[2].get();
        hypothetical_team[3] = current_team[3].get();

        double score = 0;
        size_t count = 0;
        for (const auto& mon : all_rental_pokemon()){
            if (state.seen.find(mon.first) != state.seen.end()){
                continue;
            }
            logger.log(std::to_string(count));
            hypothetical_team[c] = &mon.second;
            score += evaluate_hypothetical_team(
                state,
                hypothetical_team,
                rental_candidates_on_path,
                boss_candidates_on_path
            );
            count++;
        }
//        cout << "total score = " << score << endl;
//        cout << "candidates  = " << rental_candidates_on_path.size() << endl;
        score /= count;

        rank.emplace(score, c);
    }
#endif
    for (int8_t c = -1; c < 4; c++){
        const Pokemon* hypothetical_team[4];
        hypothetical_team[0] = current_team[0].get();
        hypothetical_team[1] = current_team[1].get();
        hypothetical_team[2] = current_team[2].get();
        hypothetical_team[3] = current_team[3].get();
        if (c >= 0){
            hypothetical_team[c] = average_rental;
        }
        double score = evaluate_hypothetical_team(
            state,
            hypothetical_team,
            rental_candidates_on_path,
            boss_candidates_on_path
        );
        rank.emplace(score, c);
    }


    if (rank.empty()){
        logger.log("Unable to compute decisions. Taking if you have the least HP.", COLOR_RED);
        return swap_if_least_hp(state, player_index);
    }

    std::string dump = "Selection Score:\n";
    for (const auto& item : rank){
        dump += std::to_string(item.first) + " : ";
        if (item.second < 0){
            dump += "Nobody";
        }else{
            dump += "Player " + std::to_string(item.second);
        }
        dump += "\n";
    }
    logger.log(dump);

    return rank.begin()->second == (int8_t)player_index;


//    return swap_if_least_hp(state, player_index);
}



}
}
}
}
