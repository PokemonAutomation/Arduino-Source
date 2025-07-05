/*  Let's Go Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV_LetsGoTools.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



EncounterRateTracker::EncounterRateTracker()
    : m_start_time(current_time())
{}
bool EncounterRateTracker::get_encounters_in_window(
    size_t& kills, size_t& encounters,
    std::chrono::seconds time_window
){
    WallClock now = current_time();
    WallClock threshold = now - time_window;

    size_t index = std::lower_bound(m_kill_history.begin(), m_kill_history.end(), threshold) - m_kill_history.begin();
//    cout << "kills index = " << index << endl;
    kills = m_kill_history.size() - index;

    index = std::lower_bound(m_encounter_history.begin(), m_encounter_history.end(), threshold) - m_encounter_history.begin();
//    cout << "encounters index = " << index << endl;
    encounters = m_encounter_history.size() - index;

//    cout << "time_window: " << std::chrono::duration_cast<std::chrono::seconds>(time_window).count() << endl;
//    cout << "m_start_time: " << std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count() << endl;

    return m_start_time <= threshold;
}
void EncounterRateTracker::report_start(WallClock now){
    m_start_time = now;
}
void EncounterRateTracker::report_kill(){
    WallClock now = current_time();
    if (m_kill_history.empty() || m_kill_history.back() < now){
        m_kill_history.push_back(now);
        return;
    }

    global_logger_tagged().log(
        "EncounterRateTracker: Detected that time has travelled backwards. Clearing history.",
        COLOR_RED
    );

    m_kill_history.clear();
    m_encounter_history.clear();
    m_start_time = now;

    m_kill_history.push_back(now);
}
void EncounterRateTracker::report_encounter(){
    WallClock now = current_time();
    if (m_encounter_history.empty() || m_encounter_history.back() < now){
        m_encounter_history.push_back(now);
        return;
    }

    global_logger_tagged().log(
        "EncounterRateTracker: Detected that time has travelled backwards. Clearing history.",
        COLOR_RED
    );

    m_kill_history.clear();
    m_encounter_history.clear();
    m_start_time = now;

    m_encounter_history.push_back(now);
}




WallDuration DiscontiguousTimeTracker::last_window_in_realtime(
    WallClock realtime_end,
    WallDuration last_window_in_virtual_time
){
    if (m_blocks.empty()){
        return WallDuration::zero();
    }

    WallDuration remaining(last_window_in_virtual_time);

    auto iter = m_blocks.rbegin();
    WallClock start;

    do{
        WallDuration block = iter->second - iter->first;
        if (remaining > block){
            remaining -= block;
            start = iter->first;
        }else{
            start = iter->second - remaining;
            return realtime_end - start;
        }
        ++iter;
    }while (iter != m_blocks.rend());

    return WallDuration::zero();
}

void DiscontiguousTimeTracker::add_block(WallClock start, WallClock end){
    if (end <= start){
        global_logger_tagged().log(
            "DiscontiguousTimeTracker: Dropping invalid time block.",
            COLOR_RED
        );
        return;
    }
    if (m_blocks.empty() || start >= m_blocks.back().second){
        m_blocks.emplace_back(start, end);
        return;
    }
    global_logger_tagged().log(
        "DiscontiguousTimeTracker: Detected that time has travelled backwards. Clearing history.",
        COLOR_RED
    );
    m_blocks.clear();
    return;
}






LetsGoEncounterBotTracker::LetsGoEncounterBotTracker(
    ProgramEnvironment& env,
    VideoStream& stream,
    LetsGoEncounterBotStats& stats,
    LetsGoKillSoundDetector& kill_sound
)
    : m_kill_sound(kill_sound)
{
    m_kill_sound.set_detected_callback([&](float){
        stream.log("Detected kill.");
        m_encounter_rate.report_kill();
        stats.m_kills++;
        env.update_stats();
        return false;
    });
}





bool use_lets_go_to_clear_in_front(
    VideoStream& stream, ProControllerContext& context,
    LetsGoEncounterBotTracker& tracker,
    bool throw_ball_if_bubble,
    std::function<void(ProControllerContext& context)>&& command
){
//    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

//    static int calls = 0;
    stream.log("Clearing what's in front with Let's Go...");
//    cout << calls++ << endl;

    SweatBubbleWatcher bubble(COLOR_GREEN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_press_button(context, BUTTON_R, 20, 200);
        },
        {bubble}
    );
//    cout << "asdf" << endl;
    if (ret == 0){
        if (throw_ball_if_bubble){
            stream.log("Detected sweat bubble. Throwing ball...");
            pbf_mash_button(context, BUTTON_ZR, 5 * TICKS_PER_SECOND);
        }else{
            stream.log("Detected sweat bubble. Will not throw ball.");
        }
    }else{
        stream.log("Did not detect sweat bubble.");
    }

    WallClock last_kill = tracker.last_kill();
    context.wait_for_all_requests();
    std::chrono::seconds timeout(6);
    while (true){
        if (command){
//            cout << "running command..." << endl;
            command(context);
            context.wait_for_all_requests();
            command = nullptr;
        }else{
//            cout << "Waiting out... " << timeout.count() << " seconds" << endl;
            context.wait_until(last_kill + timeout);
        }
//        timeout = std::chrono::seconds(3);
        if (last_kill == tracker.last_kill()){
//            cout << "no kill" << endl;
            break;
        }
//        cout << "found kill" << endl;
        last_kill = tracker.last_kill();
    }
    stream.log("Nothing left to clear...");
    tracker.throw_if_no_sound();
    return tracker.last_kill() != WallClock::min();
}

















}
}
}
