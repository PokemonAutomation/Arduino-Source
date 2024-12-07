/*  Let's Go Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
//#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
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
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    LetsGoEncounterBotStats& stats,
    OCR::LanguageOCROption& language
)
    : m_env(env)
    , m_console(console)
    , m_context(context)
    , m_stats(stats)
    , m_language(language)
    , m_kill_sound(console, [&](float){
        console.log("Detected kill.");
        m_encounter_rate.report_kill();
        stats.m_kills++;
        env.update_stats();
        return false;
    })
    , m_session(context, console, {m_kill_sound})
{}
void LetsGoEncounterBotTracker::process_battle(
    bool& caught, bool& should_save,
    EncounterWatcher& watcher, EncounterBotCommonOptions& settings
){
    m_encounter_rate.report_encounter();
    m_stats.m_encounters++;

    Language language = m_language;

    std::set<std::string> slugs;
    if (language != Language::None){
        slugs = read_singles_opponent(m_env.program_info(), m_console, m_context, language);
        m_encounter_frequencies += slugs;
        m_env.log(m_encounter_frequencies.dump_sorted_map("Encounter Stats:\n"));
    }

    bool is_shiny = (bool)watcher.shiny_screenshot();
    if (is_shiny){
        m_stats.m_shinies++;
        if (settings.VIDEO_ON_SHINY){
            m_context.wait_for(std::chrono::seconds(3));
            pbf_press_button(m_context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
        }
    }
    m_env.update_stats();

    send_encounter_notification(
        m_env,
        settings.NOTIFICATION_NONSHINY,
        settings.NOTIFICATION_SHINY,
        language != Language::None,
        is_shiny,
        {{slugs, is_shiny ? ShinyType::UNKNOWN_SHINY : ShinyType::NOT_SHINY}},
        watcher.lowest_error_coefficient(),
        watcher.shiny_screenshot(),
        &m_encounter_frequencies
    );

    //  Set default action: stop program if shiny, otherwise run away.
    EncounterActionsEntry action;
    action.action = is_shiny
        ? EncounterActionsAction::STOP_PROGRAM
        : EncounterActionsAction::RUN_AWAY;

    //  Iterate the actions table. If found an entry matches the pokemon species,
    //  set the action to be what specified in the entry.
    for (EncounterActionsEntry& entry : settings.ACTIONS_TABLE.snapshot()){
        if (language == Language::None){
            throw UserSetupError(m_console, "You must set the game language to use the actions table.");
        }

        //  See if Pokemon name matches.
        auto iter = slugs.find(entry.pokemon);
        if (iter == slugs.end()){
            continue;
        }

        switch (entry.shininess){
        case EncounterActionsShininess::ANYTHING:
            break;
        case EncounterActionsShininess::NOT_SHINY:
            if (is_shiny){
                continue;
            }
            break;
        case EncounterActionsShininess::SHINY:
            if (!is_shiny){
                continue;
            }
            break;
        }

        action = std::move(entry);
    }

    //  Run the chosen action.
    switch (action.action){
    case EncounterActionsAction::RUN_AWAY:
        try{
            run_from_battle(m_env.program_info(), m_console, m_context);
        }catch (OperationFailedException& e){
            throw FatalProgramException(std::move(e));
        }
        caught = false;
        should_save = false;
        return;
    case EncounterActionsAction::STOP_PROGRAM:
        throw ProgramFinishedException();
//        throw ProgramFinishedException(m_console, "", true);
    case EncounterActionsAction::THROW_BALLS:
    case EncounterActionsAction::THROW_BALLS_AND_SAVE:
        if (language == Language::None){
            throw InternalProgramError(&m_console.logger(), PA_CURRENT_FUNCTION, "Language is not set.");
        }

        CatchResults result = basic_catcher(
            m_console, m_context,
            language,
            action.ball, action.ball_limit,
            settings.USE_FIRST_MOVE_IF_CANNOT_THROW_BALL
        );
        send_catch_notification(
            m_env,
            settings.NOTIFICATION_CATCH_SUCCESS,
            settings.NOTIFICATION_CATCH_FAILED,
            &slugs,
            action.ball,
            result.balls_used,
            result.result
        );

        switch (result.result){
        case CatchResult::POKEMON_CAUGHT:
        case CatchResult::POKEMON_FAINTED:
            break;
        default:
            throw_and_log<FatalProgramException>(
                m_console, ErrorReport::NO_ERROR_REPORT,
                "Unable to recover from failed catch.",
                m_console
            );
        }

        caught = result.result == CatchResult::POKEMON_CAUGHT;
        should_save = caught && action.action == EncounterActionsAction::THROW_BALLS_AND_SAVE;
        return;
    }

    caught = false;
    should_save = false;
}







bool use_lets_go_to_clear_in_front(
    ConsoleHandle& console, BotBaseContext& context,
    LetsGoEncounterBotTracker& tracker,
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
    console.log("Nothing left to clear...");
    tracker.throw_if_no_sound();
    return tracker.last_kill() != WallClock::min();
}

















}
}
}
