/*  Max Lair State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_State_H
#define PokemonAutomation_PokemonSwSh_MaxLair_State_H

#include <string>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


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


struct PlayerState{
    using time_point = std::chrono::system_clock::time_point;

    int8_t console_id = -1;                 //  -1 means NPC
    std::string pokemon;
    std::string item;
    TimestampedValue<double> health = -1;   //  -1 means unknown
    int8_t pp[4] = {-1, -1, -1, -1};        //  -1 means unknown

    //  Battle State
    int8_t move_blocked[4] = {-1, -1, -1, -1};  //  -1 = unknown, 0 = unblocked, 1 = blocked

    void clear_battle_state();

    std::string dump() const;
};
struct GlobalState{
    using time_point = std::chrono::system_clock::time_point;

    std::string boss;
    uint8_t wins = 0;
    int8_t lives_left = -1; //  -1 means unknown
    PlayerState players[4];

    //  Battle State
    std::string opponent;
    TimestampedValue<double> opponent_hp = -1;
    uint8_t move_slot = 0;


    size_t find_player_index(size_t console_id) const;
    void clear_battle_state();

    std::string dump() const;
};



class GlobalStateTracker{
    using time_point = std::chrono::system_clock::time_point;
public:
    GlobalStateTracker(size_t consoles);

    //  Access the local copy for this console.
    //  This one is safe to directly access.
    GlobalState& operator[](size_t index){
        return m_consoles[index];
    }

    //  Push the local copy into the master shared copy.
    void push_update(size_t index);

    //  Get the inferred state from all the known states.
    GlobalState infer_actual_state(size_t index);

    //  Attempt to synchronize with other consoles.
    GlobalState synchronize(
        size_t index,
        std::chrono::milliseconds window = std::chrono::seconds(10)
    );


    std::string dump();


private:
    //  Recompute the group assignments.
    void update_groups_unprotected();
    bool group_is_up_to_date(uint8_t group, time_point time_min);
    GlobalState infer_actual_state_unprotected(size_t index);


private:
    size_t m_count;
    GlobalState m_consoles[4];

    std::mutex m_lock;
    std::condition_variable m_cv;

    time_point m_last_report[4];
    GlobalState m_master_consoles[4];

//    time_point m_last_group_synch[4];
    uint8_t m_groups[4];
};








}
}
}
}
#endif
