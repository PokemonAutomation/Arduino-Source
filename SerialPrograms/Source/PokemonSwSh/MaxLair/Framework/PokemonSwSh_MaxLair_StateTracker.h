/*  Max Lair State Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_StateTracker_H
#define PokemonAutomation_PokemonSwSh_MaxLair_StateTracker_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/CancellableScope.h"
#include "PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class GlobalStateTracker final : public Cancellable{
    using time_point = WallClock;
public:
    GlobalStateTracker(CancellableScope& scope, size_t consoles);
    virtual ~GlobalStateTracker();
    virtual bool cancel(std::exception_ptr exception) noexcept override;

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
        Logger& logger, size_t index,
        std::chrono::milliseconds window = std::chrono::seconds(5)
    );

    void mark_as_dead(size_t index);

    std::pair<uint64_t, std::string> dump();



private:
    //  Recompute the group assignments.
    void update_groups_unprotected();
    bool group_is_up_to_date(uint8_t group, time_point time_min);
    bool group_sync_completed(uint8_t group);
    void group_clear_status(uint8_t group);
    GlobalState infer_actual_state_unprotected(size_t index);


private:
    void merge_timestamp                (uint8_t group, GlobalState& state);
    void merge_boss                     (uint8_t group, GlobalState& state);
//    void merge_boss_type                (uint8_t group, GlobalState& state);
    void merge_path                     (uint8_t group, GlobalState& state);
    void merge_path_side                (uint8_t group, GlobalState& state);
    void merge_seen                     (uint8_t group, GlobalState& state);

    void merge_wins                     (uint8_t group, GlobalState& state);
    void merge_opponent_species         (uint8_t group, GlobalState& state);
    void merge_opponent_hp              (uint8_t group, GlobalState& state);

    void merge_player_console_id        (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_species           (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_item              (uint8_t group, PlayerState& player, size_t player_index);
//    void merge_player_dead              (uint8_t group, PlayerState& player, size_t player_index, time_point now);
//    void merge_player_hp                (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_health            (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_dmax_turns_left   (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_can_dmax          (uint8_t group, PlayerState& player, size_t player_index);
    void merge_player_pp                (uint8_t group, PlayerState& player, size_t player_index, size_t move_index);
    void merge_player_move_blocked      (uint8_t group, PlayerState& player, size_t player_index, size_t move_index);


private:
    std::mutex m_lock;
    std::condition_variable m_cv;

    uint64_t m_state_epoch = 0;

    size_t m_count;
    GlobalState m_consoles[4];

//    SpinLock m_lock;

    enum class SyncState{
        NOT_CALLED,
        WAITING,
        DONE
    };

    SyncState m_pending_sync[4] = {
        SyncState::NOT_CALLED,
        SyncState::NOT_CALLED,
        SyncState::NOT_CALLED,
        SyncState::NOT_CALLED
    };
    GlobalState m_master_consoles[4];

    uint8_t m_groups[4];
};



}
}
}
}
#endif
