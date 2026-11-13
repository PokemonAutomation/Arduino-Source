/*  Max Lair State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_State_H
#define PokemonAutomation_PokemonSwSh_MaxLair_State_H

#include <string>
#include <vector>
#include <set>
#include "Common/Cpp/Time.h"
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
        : timestamp(WallClock::min())
        , value()
    {}
    TimestampedValue(Type p_value)
        : timestamp(current_time())
        , value(std::move(p_value))
    {}
    TimestampedValue(Type p_value, WallClock timestamp)
        : timestamp(timestamp)
        , value(std::move(p_value))
    {}
    void operator=(Type x){
        timestamp = current_time();
        value = std::move(x);
    }

    operator Type() const{
        return value;
    }

    WallClock timestamp;
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



/*
  Path 0
     X
   // \\
  X X X X
  \/ \\ /
   O   O
  /\\   \
  X X X X
   \//\/
    X  X
     \/
     X

  Path 1
     X
   // \\
  X X X X
  \ //\/
   O   O
  / \ / \
  X X X X
   \/\\/
    X  X
     \/
     X

  Path 2
     X
   // \\
  X X X X
  \/ \\ /
   O   O
  /  // \
  X X X X
   \/ \/
    X  X
     \/
     X
*/

struct PathMap{
    int8_t path_type = -1;
    PokemonType boss = PokemonType::NONE;
    PokemonType mon3[4] = {PokemonType::NONE, PokemonType::NONE, PokemonType::NONE, PokemonType::NONE};
    PokemonType mon2[4] = {PokemonType::NONE, PokemonType::NONE, PokemonType::NONE, PokemonType::NONE};
    PokemonType mon1[2] = {PokemonType::NONE, PokemonType::NONE};

    std::string dump() const;
};

struct PathNode{
    uint8_t path_slot;
    PokemonType type;
};
std::string dump_path(const std::vector<PathNode>& path);


struct GlobalState{
    //  Timestamp when this state was issued.
    WallClock timestamp = WallClock::min();

    bool adventure_started = false;

    //  The adventure's boss as known from the start: the den's boss sprite,
    //  voted across consoles. Used all adventure long (path and move choice,
    //  the boss ball, the fourth battle's check, Save Path) and kept after a
    //  lost adventure.
    std::string boss;

    PathMap path;
    int8_t path_side = -1;  //  -1 = unknown, 0 = left, 1 = right
    std::vector<PathNode> last_best_path;

    uint8_t wins = 0;
    int8_t lives_left = -1; //  -1 means unknown

    PlayerState players[4];

    std::set<std::string> seen;
    void add_seen(const std::string& mon);

    //  Battle State
    std::set<std::string> opponent;
    TimestampedValue<double> opponent_hp = -1;
    uint8_t move_slot = 0;

    size_t find_player_index(size_t console_id) const;
    void clear_battle_state();

    //  Whether the opponent read fits this battle stage: before the fourth
    //  battle, any non-boss; at the fourth battle, exactly the known boss, or a
    //  single boss species when no boss is known yet.
    bool opponent_fits_stage() const;

    //  Returns false when the read doesn't fit a battle before the fourth (a
    //  boss there), so the caller retries the read. At the fourth battle,
    //  records the boss if none is known and the read fits.
    bool accept_opponent();

    //  The same boss, but only if the adventure caught it, else empty: for the
    //  end-of-adventure decision (end_adventure_action()), which must not apply
    //  the boss-caught rules after a loss. Decided from the battles won (the
    //  boss is the fourth battle), not from how many Pokemon were caught, so it
    //  stays right if non-boss catches are ever skipped.
    std::string boss_if_caught() const;

    std::string dump() const;
};







}
}
}
}
#endif
