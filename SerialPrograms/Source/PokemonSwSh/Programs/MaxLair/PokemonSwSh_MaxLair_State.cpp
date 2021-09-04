/*  Max Lair State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exception.h"
#include "PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string PlayerState::dump() const{
    std::string str;
    str += console_id < 0 ? "NPC" : " Switch " + std::to_string(console_id);
    str += " - mon:";
    str += pokemon.empty() ? "?" : pokemon;
    str += " - item:";
    str += item.empty() ? "?" : item;
    str += " - HP:";
    str += health < 0 ? "?" : std::to_string(health);
    str += "% - PP:{";
    for (size_t c = 0; c < 4; c++){
        str += std::to_string(pp[c]);
        if (c + 1 < 4){
            str += ", ";
        }
    }
    str += "} - blocked:{";
    for (size_t c = 0; c < 4; c++){
        str += std::to_string(move_blocked[c]);
        if (c + 1 < 4){
            str += ", ";
        }
    }
    str += "}";
    return str;
}
void PlayerState::clear_battle_state(){
    for (size_t c = 0; c < 4 ; c++){
        move_blocked[c] = false;
    }
}
std::string GlobalState::dump() const{
    std::string str;
    str += "    boss:";
    str += boss.empty() ? "?" : boss;
    str += " - wins:" + std::to_string(wins);
    str += " - lives:";
    str += lives_left < 0 ? "?" : std::to_string(lives_left);
    str += " - opponent:";
    str += opponent.empty() ? "?" : opponent;
    str += " - HP:";
    str += opponent_hp < 0 ? "?" : std::to_string(100 * opponent_hp);
    str += "% - slot:";
    str += std::to_string(move_slot);
    str += "\n";
    str += "    Player 0: " + players[0].dump() + "\n";
    str += "    Player 1: " + players[1].dump() + "\n";
    str += "    Player 2: " + players[2].dump() + "\n";
    str += "    Player 3: " + players[3].dump() + "\n";
    return str;
}
void GlobalState::clear_battle_state(){
    opponent.clear();
    opponent_hp = -1;
    move_slot = 0;
    players[0].clear_battle_state();
    players[1].clear_battle_state();
    players[2].clear_battle_state();
    players[3].clear_battle_state();
}
size_t GlobalState::find_player_index(size_t console_id) const{
    for (size_t c = 0; c < 4; c++){
        if (players[c].console_id == (int8_t)console_id){
            return c;
        }
    }
    return console_id;
}


GlobalStateTracker::GlobalStateTracker(size_t consoles)
    : m_count(consoles)
{
    auto timestamp = std::chrono::system_clock::now();
    for (size_t c = 0; c < 4; c++){
        m_last_report[0] = timestamp;
    }
    update_groups_unprotected();
}
std::string GlobalStateTracker::dump(){
    std::lock_guard<std::mutex> lg(m_lock);
    std::string str;
    for (size_t c = 0; c < m_count; c++){
        str += "Switch " + std::to_string(c) + ":\n";
        str += m_consoles[c].dump();
    }
#if 0
    for (size_t c = 0; c < m_count; c++){
        str += "Inferred " + std::to_string(c) + ":\n";
        str += compute_inferred_unprotected(c).dump();
    }
#endif
    return str;
}
void GlobalStateTracker::push_update(size_t index){
    std::lock_guard<std::mutex> lg(m_lock);
    m_master_consoles[index] = m_consoles[index];
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

        //  Last update is too old.
        if (m_last_report[c] < time_min){
            return false;
        }
    }
    return true;
}

GlobalState GlobalStateTracker::synchronize(size_t index, std::chrono::milliseconds window){
    std::unique_lock<std::mutex> lg(m_lock);
    m_master_consoles[index] = m_consoles[index];
    m_cv.notify_all();

#if 0
    time_point timestamp = std::chrono::system_clock::now();
    time_point time_limit = timestamp + window;
    m_last_report[index] = timestamp;

//    std::unique_lock<std::mutex> lg(m_lock);
    while (true){
        time_point now = std::chrono::system_clock::now();
        if (now > time_limit){
            break;
        }

        uint8_t group = m_groups[index];
        if (group_is_up_to_date(group, now - window)){
            break;
        }

        m_cv.wait_until(lg, time_limit);
    }

    update_groups_unprotected();

#endif

//    return m_consoles[index];
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
    Type best_value;
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
    Type best_value;
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
    TimestampedValue<double> latest(-1, std::chrono::system_clock::time_point::min());
    for (size_t c = 0; c < count; c++){
        if (group != groups[c]){
            continue;
        }
        const TimestampedValue<double>& current = lambda(c);
        if (latest.timestamp < current.timestamp){
            latest = current;
        }
    }
    return latest;
}

GlobalState GlobalStateTracker::infer_actual_state_unprotected(size_t index){
    uint8_t group = m_groups[index];

    GlobalState state;

    state.boss = merge_majority_vote<std::string>(
        m_groups, m_count, group, "",
        [&](size_t index){ return m_master_consoles[index].boss; }
    );
    state.wins = merge_majority_vote<uint8_t>(
        m_groups, m_count, group,
        [&](size_t index){ return m_master_consoles[index].wins; }
    );
    state.opponent = merge_majority_vote<std::string>(
        m_groups, m_count, group, "",
        [&](size_t index){ return m_master_consoles[index].opponent; }
    );
    state.opponent_hp = merge_hp(
        m_groups, m_count, group,
        [&](size_t index){ return m_master_consoles[index].opponent_hp; }
    );

    for (size_t player_index = 0; player_index < 4; player_index++){
        PlayerState& player = state.players[player_index];
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
        player.pokemon = merge_majority_vote<std::string>(
            m_groups, m_count, group, "",
            [&](size_t index){ return m_master_consoles[index].players[player_index].pokemon; }
        );
        player.item = merge_majority_vote<std::string>(
            m_groups, m_count, group, "",
            [&](size_t index){ return m_master_consoles[index].players[player_index].item; }
        );
        for (size_t c = 0; c < 4; c++){
            player.pp[c] = merge_majority_vote<int8_t>(
                m_groups, m_count, group, -1,
                [&](size_t index){ return m_master_consoles[index].players[player_index].pp[c]; }
            );
            player.move_blocked[c] = merge_majority_vote<int8_t>(
                m_groups, m_count, group, -1,
                [&](size_t index){ return m_master_consoles[index].players[player_index].move_blocked[c]; }
            );
        }
    }

    return state;
}
void GlobalStateTracker::update_groups_unprotected(){
    m_groups[0] = 0;
    m_groups[1] = 0;
    m_groups[2] = 0;
    m_groups[3] = 0;
}









}
}
}
}
