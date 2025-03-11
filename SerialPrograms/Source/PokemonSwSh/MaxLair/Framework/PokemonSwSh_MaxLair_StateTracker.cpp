/*  Max Lair State Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <mutex>
#include <condition_variable>
#include "Common/Compiler.h"
#include "Common/Cpp/AbstractLogger.h"
#include "PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


GlobalStateTracker::GlobalStateTracker(CancellableScope& scope, size_t consoles)
    : m_count(consoles)
{
    auto timestamp = current_time();
    for (size_t c = 0; c < 4; c++){
        m_master_consoles[c].timestamp = timestamp;
    }
    update_groups_unprotected();
    attach(scope);
}
GlobalStateTracker::~GlobalStateTracker(){
    detach();
}
std::pair<uint64_t, std::string> GlobalStateTracker::dump(){
    std::lock_guard<std::mutex> lg(m_lock);
    std::string str;
    for (size_t c = 0; c < m_count; c++){
        str += "Switch " + std::to_string(c) + ": (group " + std::to_string(m_groups[c]) + ")\n";
        str += m_consoles[c].dump();
    }
#if 0
    for (size_t c = 0; c < m_count; c++){
        str += "Inferred " + std::to_string(c) + ":\n";
        str += compute_inferred_unprotected(c).dump();
    }
#endif
    return {m_state_epoch, str};
}
bool GlobalStateTracker::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(std::move(exception))){
        return true;
    }
    std::lock_guard<std::mutex> lg(m_lock);
    m_cv.notify_all();
    return false;
}
void GlobalStateTracker::push_update(size_t index){
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_epoch++;
    m_master_consoles[index] = m_consoles[index];
    m_master_consoles[index].timestamp = current_time();
    m_cv.notify_all();
}

GlobalState GlobalStateTracker::infer_actual_state(size_t index){
    std::lock_guard<std::mutex> lg(m_lock);
    return infer_actual_state_unprotected(index);
}

bool GlobalStateTracker::group_is_up_to_date(uint8_t group, time_point time_min){
    for (size_t c = 0; c < m_count; c++){
        //  Skip non-matching groups.
        if (m_groups[c] != group){
            continue;
        }
        if (m_pending_sync[c] == SyncState::NOT_CALLED){
            return false;
        }
    }
    return true;
}
bool GlobalStateTracker::group_sync_completed(uint8_t group){
    for (size_t c = 0; c < m_count; c++){
        //  Skip non-matching groups.
        if (m_groups[c] != group){
            continue;
        }
        if (m_pending_sync[c] != SyncState::DONE){
            return false;
        }
    }
    return true;
}
void GlobalStateTracker::group_clear_status(uint8_t group){
    for (size_t c = 0; c < m_count; c++){
        //  Skip non-matching groups.
        if (m_groups[c] != group){
            continue;
        }
        m_pending_sync[c] = SyncState::NOT_CALLED;
    }
}
void GlobalStateTracker::mark_as_dead(size_t index){
    std::lock_guard<std::mutex> lg(m_lock);
    m_state_epoch++;
    m_groups[index] = 4;
}

GlobalState GlobalStateTracker::synchronize(
    Logger& logger, size_t index, std::chrono::milliseconds window
){
    std::unique_lock<std::mutex> lg(m_lock);
    m_state_epoch++;

    time_point timestamp = current_time();
    logger.log("Synchronizing...", COLOR_MAGENTA);

    m_master_consoles[index] = m_consoles[index];
    m_master_consoles[index].timestamp = timestamp;
    m_pending_sync[index] = SyncState::WAITING;

    if (group_is_up_to_date(m_groups[index], timestamp - window)){
        //  We're the last one. Wake everyone up and return result.
        m_cv.notify_all();
        m_pending_sync[index] = SyncState::DONE;
        return infer_actual_state_unprotected(index);
    }

    //  Not the last one. Need to wait.
    time_point time_limit = timestamp + window;
    while (true){
        m_cv.wait_until(lg, time_limit);

        throw_if_cancelled();

        time_point now = current_time();
        if (now > time_limit){
            m_state_epoch++;
            logger.log("GlobalStateTracker::synchronize() timed out.", COLOR_RED);
            group_clear_status(m_groups[index]);
            update_groups_unprotected();
            return infer_actual_state_unprotected(index);
        }

        if (group_is_up_to_date(m_groups[index], now - window)){
            break;
        }
    }

    m_state_epoch++;
    m_pending_sync[index] = SyncState::DONE;

    if (!group_sync_completed(m_groups[index])){
        //  Not the last one out. Just return.
        return infer_actual_state_unprotected(index);
    }

    //  Last one out, clear and return.
    group_clear_status(m_groups[index]);
    update_groups_unprotected();
    return infer_actual_state_unprotected(index);
}




//
//  This is more-or-less AI code for detecting forks and merging discrepancies.
//


template <typename Type, typename Lambda>
Type merge_majority_vote(
    const uint8_t groups[4],
    size_t count, uint8_t group,
    Lambda&& lambda
){
    std::map<Type, size_t> map;
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        const Type& value = lambda(c);
        map[value]++;
    }
    size_t best_count = 0;
    Type best_value{};
    for (const auto& item : map){
        if (best_count <= item.second){
            best_count = item.second;
            best_value = item.first;
        }
    }
    return best_value;
}
template <typename Type, typename Lambda>
Type merge_majority_vote(
    const uint8_t groups[4],
    size_t count, uint8_t group, Type null_value,
    Lambda&& lambda
){
    std::map<Type, size_t> map;
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        const Type& value = lambda(c);
        if (value != null_value){
            map[value]++;
        }
    }
    size_t best_count = 0;
    Type best_value = null_value;
    for (const auto& item : map){
        if (best_count <= item.second){
            best_count = item.second;
            best_value = item.first;
        }
    }
    return best_value;
}
template <typename Type, typename Lambda>
Type merge_majority_vote(
    const uint8_t groups[4],
    size_t count, uint8_t group, Type null_value,
    GlobalState states[4],
    WallClock min_valid_time,
    Lambda&& lambda
){
    std::map<Type, size_t> map;
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        const Type& value = lambda(c);
        if (value != null_value && states[c].timestamp >= min_valid_time){
            map[value]++;
        }
    }
    size_t best_count = 0;
    Type best_value = null_value;
    for (const auto& item : map){
        if (best_count <= item.second){
            best_count = item.second;
            best_value = item.first;
        }
    }
    return best_value;
}
template <typename Lambda>
TimestampedValue<double> merge_hp(
    const uint8_t groups[4],
    size_t count, uint8_t group,
    Lambda&& lambda
){
#if 0
    double sum = 0;
    size_t num = 0;
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        double value = lambda(c);
        if (value < 0){
            continue;
        }
        sum += value;
        num++;
    }
    return num == 0 ? -1 : sum / num;
#endif
    TimestampedValue<double> latest(-1, WallClock::min());
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        const TimestampedValue<double>& current = lambda(c);
        if (latest.timestamp < current.timestamp && current >= 0){
            latest = current;
        }
    }
    return latest;
}



std::set<std::string> merge_sets(const std::vector<const std::set<std::string>*>& sets){
    std::map<std::string, size_t> map;
    for (const std::set<std::string>* set : sets){
        if (set == nullptr){
            continue;
        }
        for (const std::string& str : *set){
            map[str]++;
        }
    }
    if (map.empty()){
        return std::set<std::string>();
    }
    std::multimap<size_t, std::string, std::greater<size_t>> sorted;
    for (const auto& item : map){
        sorted.emplace(item.second, item.first);
    }
    size_t count = sorted.begin()->first;
    std::set<std::string> ret;
    for (const auto& item : sorted){
        if (item.first != count){
            break;
        }
        ret.insert(item.second);
    }
    return ret;
}

void GlobalStateTracker::merge_timestamp(uint8_t group, GlobalState& state){
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            continue;
        }
        state.timestamp = std::max(state.timestamp, m_master_consoles[c].timestamp);
    }
}
void GlobalStateTracker::merge_boss(uint8_t group, GlobalState& state){
    state.boss = merge_majority_vote<std::string>(
        m_groups, m_count, group, "",
        [&](size_t index){ return m_master_consoles[index].boss; }
    );
}
#if 0
void GlobalStateTracker::merge_boss_type(uint8_t group, GlobalState& state){
    state.boss_type = merge_majority_vote<PokemonType>(
        m_groups, m_count, group, PokemonType::NONE,
        [&](size_t index){ return m_master_consoles[index].boss_type; }
    );
}
#endif
void GlobalStateTracker::merge_path(uint8_t group, GlobalState& state){
//    WallClock latest =  WallClock::min();
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            continue;
        }
        if (m_master_consoles[c].path.path_type >= 0){
            state.path = m_master_consoles[c].path;
            state.last_best_path = m_master_consoles[c].last_best_path;
            return;
        }
    }
}
void GlobalStateTracker::merge_path_side(uint8_t group, GlobalState& state){
    state.path_side = merge_majority_vote<int8_t>(
        m_groups, m_count, group, -1,
        [&](size_t index){ return m_master_consoles[index].path_side; }
    );
}
void GlobalStateTracker::merge_seen(uint8_t group, GlobalState& state){
    std::set<std::string> merged;
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            continue;
        }
        for (const std::string& mon : m_master_consoles[c].seen){
            merged.insert(mon);
        }
    }
    state.seen = std::move(merged);
}
void GlobalStateTracker::merge_wins(uint8_t group, GlobalState& state){
    state.wins = merge_majority_vote<uint8_t>(
        m_groups, m_count, group,
        [&](size_t index){ return m_master_consoles[index].wins; }
    );
}
void GlobalStateTracker::merge_opponent_species(uint8_t group, GlobalState& state){
    state.opponent = merge_sets({
        0 < m_count && m_groups[0] == group ? &m_master_consoles[0].opponent : nullptr,
        1 < m_count && m_groups[1] == group ? &m_master_consoles[1].opponent : nullptr,
        2 < m_count && m_groups[2] == group ? &m_master_consoles[2].opponent : nullptr,
        3 < m_count && m_groups[3] == group ? &m_master_consoles[3].opponent : nullptr,
    });
}
void GlobalStateTracker::merge_opponent_hp(uint8_t group, GlobalState& state){
    state.opponent_hp = merge_hp(
        m_groups, m_count, group,
        [&](size_t index){ return m_master_consoles[index].opponent_hp; }
    );
}
void GlobalStateTracker::merge_player_species(uint8_t group, PlayerState& player, size_t player_index){
    //  Check if it's self-reported.
    do{
        if (player.console_id < 0){
            break;
        }
        if (group != m_groups[player.console_id]){
            return;
        }
        std::string self_reported_mon = m_master_consoles[player.console_id].players[player_index].pokemon;
        if (!self_reported_mon.empty()){
            player.pokemon = std::move(self_reported_mon);
            return;
        }
    }while (false);

    //  Otherwise, do majority vote.
    player.pokemon = merge_majority_vote<std::string>(
        m_groups, m_count, group, "",
        [&](size_t index){ return m_master_consoles[index].players[player_index].pokemon; }
    );
}
void GlobalStateTracker::merge_player_console_id(uint8_t group, PlayerState& player, size_t player_index){
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            player.console_id = -1;
            continue;
        }
        int8_t console_id = m_master_consoles[c].players[player_index].console_id;
        if (console_id >= 0){
            player.console_id = console_id;
            break;
        }
    }
}
void GlobalStateTracker::merge_player_item(uint8_t group, PlayerState& player, size_t player_index){
    player.item = merge_majority_vote<std::string>(
        m_groups, m_count, group, "",
        [&](size_t index){ return m_master_consoles[index].players[player_index].item; }
    );
}
#if 0
void GlobalStateTracker::merge_player_dead(uint8_t group, PlayerState& player, size_t player_index, time_point now){
    player.is_dead = merge_majority_vote<int8_t>(
        m_groups, m_count, group, -1,
        m_master_consoles, now - std::chrono::seconds(12),
        [&](size_t index){ return m_master_consoles[index].players[player_index].is_dead; }
    );
}
void GlobalStateTracker::merge_player_hp(uint8_t group, PlayerState& player, size_t player_index){
    player.hp = merge_hp(
        m_groups, m_count, group,
        [&](size_t index){ return m_master_consoles[index].players[player_index].hp; }
    );
}
#endif
void GlobalStateTracker::merge_player_health(uint8_t group, PlayerState& player, size_t player_index){
    TimestampedValue<Health> latest(Health(), WallClock::min());
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            continue;
        }
        const TimestampedValue<Health>& current = m_master_consoles[c].players[player_index].health;
        if (latest.timestamp < current.timestamp && current.value.hp >= 0){
            latest = current;
        }
    }
    player.health = latest;
}
void GlobalStateTracker::merge_player_dmax_turns_left(uint8_t group, PlayerState& player, size_t player_index){
    player.dmax_turns_left = merge_majority_vote<int8_t>(
        m_groups, m_count, group, -1,
        [&](size_t index){ return m_master_consoles[index].players[player_index].dmax_turns_left; }
    );
}
void GlobalStateTracker::merge_player_can_dmax(uint8_t group, PlayerState& player, size_t player_index){
    //  Ability to dmax is self-reporting.
    for (size_t c = 0; c < m_count; c++){
        if (group != m_groups[c]){
            continue;
        }
        player.can_dmax |= m_master_consoles[c].players[player_index].can_dmax;
    }
}
void GlobalStateTracker::merge_player_pp(uint8_t group, PlayerState& player, size_t player_index, size_t move_index){
    player.pp[move_index] = merge_majority_vote<int8_t>(
        m_groups, m_count, group, -1,
        [&](size_t index){ return m_master_consoles[index].players[player_index].pp[move_index]; }
    );
}
void GlobalStateTracker::merge_player_move_blocked(uint8_t group, PlayerState& player, size_t player_index, size_t move_index){
    player.move_blocked[move_index] = merge_majority_vote<bool>(
        m_groups, m_count, group, false,
        [&](size_t index){ return m_master_consoles[index].players[player_index].move_blocked[move_index]; }
    );
}


GlobalState GlobalStateTracker::infer_actual_state_unprotected(size_t index){
    uint8_t group = m_groups[index];

    GlobalState state;
    merge_timestamp(group, state);
    merge_boss(group, state);
    merge_path(group, state);
    merge_path_side(group, state);
    merge_seen(group, state);

    merge_wins(group, state);
    merge_opponent_species(group, state);
    merge_opponent_hp(group, state);

    for (size_t player_index = 0; player_index < 4; player_index++){
        PlayerState& player = state.players[player_index];
        merge_player_console_id(group, player, player_index);
        merge_player_species(group, player, player_index);
        merge_player_item(group, player, player_index);
//        merge_player_dead(group, player, player_index, state.timestamp);
//        merge_player_hp(group, player, player_index);
        merge_player_health(group, player, player_index);
        merge_player_dmax_turns_left(group, player, player_index);
        merge_player_can_dmax(group, player, player_index);
        for (size_t c = 0; c < 4; c++){
            merge_player_pp(group, player, player_index, c);
            merge_player_move_blocked(group, player, player_index, c);
        }
    }

    return state;
}
void GlobalStateTracker::update_groups_unprotected(){
#if 0
    //  Assume everyone's in a different universe.
    m_groups[0] = 0;
    m_groups[1] = 1;
    m_groups[2] = 2;
    m_groups[3] = 3;
#endif
#if 1
    //  Assume everyone's in the same universe.
    m_groups[0] = 0;
    m_groups[1] = 0;
    m_groups[2] = 0;
    m_groups[3] = 0;
#endif
}




}
}
}
}
