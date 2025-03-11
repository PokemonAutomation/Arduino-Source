/*  Max Lair AI Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"
#include "PokemonSwSh_MaxLair_AI_Tools.h"
#include "PokemonSwSh_MaxLair_AI_PathMatchup.h"
#include "PokemonSwSh_MaxLair_AI_RentalBossMatchup.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::unique_ptr<papkmnlib::Pokemon> convert_player_to_pkmnlib(const PlayerState& player){
    using namespace papkmnlib;
    if (player.pokemon.empty()){
        return nullptr;
    }
    std::unique_ptr<Pokemon> pokemon(new Pokemon(get_pokemon(player.pokemon)));
    pokemon->set_hp_ratio(player.health.value.dead ? 0 : player.health.value.hp);
    pokemon->set_is_dynamax(player.dmax_turns_left > 0);
    size_t move_count = std::min(pokemon->num_moves(), (size_t)4);
    for (size_t i = 0; i < move_count; i++){
        int8_t pp = player.pp[i];
        if (pp < 0){
            pp = pokemon->move(i).pp();
        }
        pokemon->update_pp(i, pp);
    }
    return pokemon;
}




std::set<std::string> get_rental_candidates_on_path(
    const std::vector<PathNode>& path,
    const std::set<std::string>& exclusions
){
    std::set<std::string> candidates;
    for (const PathNode& node : path){
        for (const std::string& rental : rentals_by_type(node.type)){
            if (exclusions.find(rental) == exclusions.end()){
                candidates.insert(rental);
            }
        }
    }
    return candidates;
}
std::set<std::string> get_rental_candidates_on_path(const GlobalState& state){
    return get_rental_candidates_on_path(state.last_best_path, state.seen);
}
std::vector<const papkmnlib::Pokemon*> get_rental_candidates_on_path_pkmnlib(const GlobalState& state){
    std::vector<const papkmnlib::Pokemon*> candidates;
    for (const std::string& candidate : get_rental_candidates_on_path(state)){
        candidates.emplace_back(&papkmnlib::get_pokemon(candidate));
    }
    return candidates;
}
std::vector<const papkmnlib::Pokemon*> get_boss_candidates(const GlobalState& state){
    using namespace papkmnlib;
    std::vector<const Pokemon*> candidates;
    if (state.boss.empty()){
        PokemonType boss_type = state.path.boss;
        Type pkmnlib_type = serial_type_to_pkmnlib(boss_type);
        for (const auto& item : all_boss_pokemon()){
            const Pokemon& boss = get_pokemon(item.first);
            if (boss_type == PokemonType::NONE || boss.has_type(pkmnlib_type)){
                candidates.emplace_back(&boss);
            }
        }
    }else{
        candidates.emplace_back(&get_pokemon(state.boss));
    }
    return candidates;
}





double rental_vs_boss_matchup(const std::string& rental, const std::vector<const papkmnlib::Pokemon*>& bosses){
    using namespace papkmnlib;

    if (bosses.empty()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Boss list cannot be empty.");
    }
    double score = 0;
    if (rental.empty()){
        for (const Pokemon* boss : bosses){
            for (const auto& candidate : all_rental_pokemon()){
                score += rental_vs_boss_matchup(candidate.first, boss->name());
            }
        }
        score /= bosses.size() * all_rental_pokemon().size();
    }else{
        for (const Pokemon* boss : bosses){
            score += rental_vs_boss_matchup(rental, boss->name());
        }
        score /= bosses.size();
    }
    return score;
}
double rental_vs_boss_matchup(const papkmnlib::Pokemon* rental, const std::vector<const papkmnlib::Pokemon*>& bosses){
    return rental_vs_boss_matchup(rental == nullptr ? "" : rental->name(), bosses);
}



double evaluate_hypothetical_team(
    const GlobalState& state,
    const papkmnlib::Pokemon* team[4],
    const std::vector<const papkmnlib::Pokemon*>& rental_candidates_on_path,
    const std::vector<const papkmnlib::Pokemon*>& boss_candidates_on_path
){
    using namespace papkmnlib;

    uint8_t lives = 4;

    double total = 0;
    for (size_t c = 0; c < 4; c++){
        double score = rental_vs_boss_matchup(team[c], boss_candidates_on_path);

        //  Adjust for HP.
        if (team[c] != nullptr){
            double hp = team[c]->hp_ratio();
            if (hp >= 0){
                score *= (hp + lives - 1) / lives;
            }
        }

        //  NPCs are stupid.
        if (state.players[c].console_id < 0){
            score *= 0.5;
        }

        total += score;
    }
    return total;
}


#if 0
double evaluate_hypothetical_team(
    const GlobalState& state,
    const papkmnlib::Pokemon* team[4],
    const std::vector<const papkmnlib::Pokemon*>& rental_candidates_on_path,
    const std::vector<const papkmnlib::Pokemon*>& boss_candidates_on_path
){
    using namespace papkmnlib;

#if 1
    uint8_t lives = 4;

    double rental_weight = 3 - state.wins;
    double boss_weight = 2;

    double sum = 0;
    size_t count = 0;

//    static std::mutex lock;
//    std::lock_guard<std::mutex> lg(lock);

//    cout << team[0]->dump() << endl;
//    cout << team[1]->dump() << endl;
//    cout << team[2]->dump() << endl;
//    cout << team[3]->dump() << endl;

    for (size_t c = 0; c < 4; c++){
//        cout << "Player: " << c << endl;
        if (state.players[c].console_id < 0){
            continue;
        }
        const Pokemon* attacker = team[c];
        if (attacker == nullptr){
            continue;
        }

        std::vector<const Pokemon*> npcs;
        for (size_t i = 0; i < 4; i++){
            if (c != i && team[i] != nullptr){
                npcs.emplace_back(team[i]);
            }
        }
//        cout << "npcs.size() = " << npcs.size() << endl;

//        auto time0 = current_time();
        double rental_score = evaluate_average_matchup(
            *attacker,
            rental_candidates_on_path,
            npcs, lives
        );
        double boss_score = evaluate_average_matchup(
            *attacker,
            boss_candidates_on_path,
            npcs, lives
        );
//        auto time1 = current_time();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

        double weighted_score = get_weighted_score(
            rental_score, rental_weight,
            boss_score, boss_weight
        );

//        cout << "rental_score   = " << rental_score << endl;
//        cout << "boss_score     = " << boss_score << endl;
//        cout << "weighted_score = " << weighted_score << endl;

        sum += weighted_score;
        count++;
    }

    if (count == 0){
        return 0;
    }
    return sum / (double)count;
#endif
}
#endif




bool swap_if_least_hp(
    const GlobalState& state,
    size_t player_index
){
    double least_hp = 1.0;
    int player = -1;

    for (int c = 0; c < 4; c++){
        double hp = state.players[c].health.value.hp;
        if (hp < 0){
            continue;
        }

        if (least_hp > hp){
            least_hp = hp;
            player = c;
        }
    }

    return (int)player_index == player;
}




}
}
}
}
