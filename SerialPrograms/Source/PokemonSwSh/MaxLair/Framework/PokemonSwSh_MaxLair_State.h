/*  Max Lair State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_State_H
#define PokemonAutomation_PokemonSwSh_MaxLair_State_H

#include <string>
#include <chrono>
#include <set>
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


template <typename Type>
struct TimestampedValue{
    TimestampedValue(const TimestampedValue& x)
        : timestamp(x.timestamp)
        , value(x.value)
    {}
    void operator=(const TimestampedValue& x){
        timestamp = x.timestamp;
        value = x.value;
    }

    TimestampedValue()
        : timestamp(std::chrono::system_clock::time_point::min())
        , value()
    {}
    TimestampedValue(Type p_value)
        : timestamp(std::chrono::system_clock::now())
        , value(std::move(p_value))
    {}
    TimestampedValue(Type p_value, std::chrono::system_clock::time_point timestamp)
        : timestamp(timestamp)
        , value(std::move(p_value))
    {}
    void operator=(Type x){
        timestamp = std::chrono::system_clock::now();
        value = std::move(x);
    }

    operator Type() const{
        return value;
    }

    std::chrono::system_clock::time_point timestamp;
    Type value;
};


struct Health{
    double hp = -1;     //  [0.0 - 1.0], -1 means unknown.
    int8_t dead = -1;   //  1 = dead, 0 = alive, -1 = unknown.
};


struct PlayerState{
    int8_t console_id = -1;             //  -1 means NPC
    std::string pokemon;
    std::string item;
    int8_t dmax_turns_left = -1;        //  -1 means unknown
    bool can_dmax = false;
    TimestampedValue<Health> health;
    int8_t pp[4] = {-1, -1, -1, -1};    //  -1 means unknown

    //  Battle State
    bool move_blocked[4] = {false, false, false, false};

    void clear_battle_state();

    std::string dump() const;
};



//  Path 0
//     X
//   // \\
//  X X X X
//  \/ \\ /
//   O   O
//  /\\   \
//  X X X X
//   \//\/
//    X  X
//     \/
//     X

//  Path 1
//     X
//   // \\
//  X X X X
//  \ //\/
//   O   O
//  / \ / \
//  X X X X
//   \/\\/
//    X  X
//     \/
//     X

//  Path 2
//     X
//   // \\
//  X X X X
//  \/ \\ /
//   O   O
//  /  // \
//  X X X X
//   \/ \/
//    X  X
//     \/
//     X

struct Path{
    uint8_t path_type = 0;
//    PokemonType boss;
    PokemonType mon3[4] = {PokemonType::NONE, PokemonType::NONE, PokemonType::NONE, PokemonType::NONE};
    PokemonType mon2[4] = {PokemonType::NONE, PokemonType::NONE, PokemonType::NONE, PokemonType::NONE};
    PokemonType mon1[2] = {PokemonType::NONE, PokemonType::NONE};

    std::string dump() const;
};


struct GlobalState{
    using time_point = std::chrono::system_clock::time_point;

    //  Timestamp when this state was issued.
    time_point timestamp = time_point::min();

    bool adventure_started = false;

    std::string boss;
    uint8_t wins = 0;
    int8_t lives_left = -1; //  -1 means unknown
    PlayerState players[4];

    Path path;

    //  Battle State
    std::set<std::string> opponent;
    TimestampedValue<double> opponent_hp = -1;
    uint8_t move_slot = 0;

    size_t find_player_index(size_t console_id) const;
    void clear_battle_state();

    std::string dump() const;
};







}
}
}
}
#endif
