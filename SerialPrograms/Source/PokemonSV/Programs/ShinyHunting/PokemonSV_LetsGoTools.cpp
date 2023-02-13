/*  Let's Go Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "PokemonSV_LetsGoTools.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



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





bool clear_in_front(
    ConsoleHandle& console, BotBaseContext& context,
    LetsGoKillSoundDetector& kill_tracker,
    bool throw_ball_if_bubble,
    std::function<void(BotBaseContext& context)>&& command
){
//    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

//    static int calls = 0;
    console.log("Clearing what's in front with Let's Go...");
//    cout << calls++ << endl;

    SweatBubbleWatcher bubble(COLOR_GREEN);
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_press_button(context, BUTTON_R, 20, 200);
        },
        {bubble}
    );
//    cout << "asdf" << endl;
    if (ret == 0){
        if (throw_ball_if_bubble){
            console.log("Detected sweat bubble. Throwing ball...");
            pbf_mash_button(context, BUTTON_ZR, 5 * TICKS_PER_SECOND);
        }else{
            console.log("Detected sweat bubble. Will not throw ball.");
        }
    }else{
        console.log("Did not detect sweat bubble.");
    }

    WallClock last_kill = kill_tracker.last_kill();
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
        if (last_kill == kill_tracker.last_kill()){
//            cout << "no kill" << endl;
            break;
        }
//        cout << "found kill" << endl;
        last_kill = kill_tracker.last_kill();
    }
    console.log("Nothing left to clear...");
    return kill_tracker.last_kill() != WallClock::min();
}





}
}
}
