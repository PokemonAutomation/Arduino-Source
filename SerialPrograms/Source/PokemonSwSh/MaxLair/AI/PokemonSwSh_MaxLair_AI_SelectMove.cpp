/*  Max Lair AI Select Move
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


double relative_damage(const MaxLairMon& you, const MaxLairMon& opponent, uint8_t move_slot, bool dmax){
    const MaxLairMove& move = dmax
        ? you.max_moves[move_slot]
        : you.moves[move_slot];

    if (move.slug.empty()){
        return -1;
    }

    double damage = move.effective_power;
//    cout << "power = " << damage << endl;
    damage *= move.accuracy;
//    cout << "power = " << damage << endl;

    //  STAB
    if (move.type == you.type[0] || move.type == you.type[1]){
        damage *= 1.5;
    }

//    cout << "power = " << damage << endl;

    if (move.category == MoveCategory::PHYSICAL){
        damage *= you.base_stats[1] * 2 + 26.6667;      //  26.6667 is the average IV.
        damage /= opponent.base_stats[2] * 2 + 26.6667;
    }
    if (move.category == MoveCategory::SPECIAL){
        damage *= you.base_stats[3] * 2 + 26.6667;
        damage /= opponent.base_stats[4] * 2 + 26.6667;
    }
//    cout << "stats = " << damage << endl;

    damage *= type_multiplier(move.type, opponent.type[0], opponent.type[1]);
//    cout << "damage = " << damage << endl;

//    cout << damage << endl;

    return damage;
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
        "purple"
    );

    const PlayerState& player = state.players[player_index];
    const MaxLairMon* self = get_maxlair_mon_nothrow(player.pokemon);
    if (self == nullptr){
        logger.log("Don't know what you are. Picking a random move...", Qt::red);
        return {random(0, 3), false};
    }

    std::multimap<double, std::pair<uint8_t, bool>, std::greater<double>> rank;
    size_t start, stop;
    if (player.dmax_turns_left > 0){
        start = 4;
        stop = 8;
    }else if (player.can_dmax){
        start = 0;
        stop = 8;
    }else{
        start = 0;
        stop = 4;
    }
    for (size_t c = start; c < stop; c++){
        uint8_t move_slot = c % 4;
        bool dmax = c >= 4;
        double damage = -1;
        for (const std::string& slug : state.opponent){
            const MaxLairMon& mon = get_maxlair_mon(slug);
            double current = relative_damage(*self, mon, move_slot, dmax);
            if (damage < 0){
                damage = current;
            }else{
                damage = std::min(damage, current);
            }
        }
        rank.emplace(damage, std::pair<uint8_t, bool>(move_slot, dmax));
    }

    std::string move_dump = "Relative Move Effectiveness:\n";
    for (const auto& move : rank){
        uint8_t slot = move.second.first;
        move_dump += std::to_string(move.first) + " : ";
        move_dump += move.second.second
            ? self->max_moves[slot].slug
            : self->moves[slot].slug;
        move_dump += "\n";
    }
    logger.log(move_dump);

    if (rank.empty()){
        logger.log("Unable to calculate moves. Picking a random move...", Qt::red);
        return {random(0, 3), false};
    }
    if (rank.begin()->first < 0){
        logger.log("No viable moves found. Picking a random move...", Qt::red);
        return {random(0, 3), false};
    }

    for (const auto& move : rank){
        uint8_t slot = move.second.first;
        if (!player.move_blocked[slot]){
            return move.second;
        }
    }

    logger.log("Unable to find a usable move. Picking a random move...", Qt::red);
    return {random(0, 3), false};
}



}
}
}
}
