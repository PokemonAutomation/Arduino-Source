/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_LetsGoTools.h"
#include "PokemonSV_ShinyHunt-AreaZeroPlatform.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



ShinyHuntAreaZeroPlatform_Descriptor::ShinyHuntAreaZeroPlatform_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ShinyHuntAreaZeroPlatform",
        STRING_POKEMON + " SV", "Shiny Hunt - Area Zero Platform",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ShinyHunt-AreaZeroPlatform.md",
        "Shiny hunt the isolated platform at the bottom of Area Zero.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct ShinyHuntAreaZeroPlatform_Descriptor::Stats : public LetsGoEncounterBotStats{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_platform_resets(m_stats["Platform Resets"])
        , m_game_resets(m_stats["Game Resets"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 2, {"Sandwiches", true});
        m_display_order.insert(m_display_order.begin() + 3, {"Platform Resets", true});
        m_display_order.insert(m_display_order.begin() + 4, {"Game Resets", true});
        m_display_order.insert(m_display_order.begin() + 5, {"Errors", true});
    }
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_platform_resets;
    std::atomic<uint64_t>& m_game_resets;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> ShinyHuntAreaZeroPlatform_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




ShinyHuntAreaZeroPlatform::ShinyHuntAreaZeroPlatform()
    : LANGUAGE(
        "<b>Game Language:</b><br>Required to read " + STRING_POKEMON + " names.",
        IV_READER().languages(),
        LockWhileRunning::UNLOCKED,
        false
    )
    , MODE(
        "<b>Mode:</b><br>"
        "If starting on the platform, you should stand near the center of the platform facing any direction.<br>"
        "If starting in the Zero Gate, you should be just inside the building as if you just entered.<br>"
//        "If making a sandwich, you should be at the Zero Gate fly spot as if you just flew there."
        ,
        {
            {Mode::START_ON_PLATFORM,   "platform", "Start on platform."},
            {Mode::START_IN_ZERO_GATE,  "zerogate", "Start inside Zero Gate."},
//            {Mode::MAKE_SANDWICH,       "sandwich", "Make a sandwich."},
        },
        LockWhileRunning::LOCKED,
        Mode::START_ON_PLATFORM
    )
    , PATH0(
        "<b>Path:</b><br>Traversal path on the platform to trigger encounters.",
        {
            {Path::PATH0, "path0", "Path 0"},
            {Path::PATH1, "path1", "Path 1"},
            {Path::PATH2, "path2", "Path 2"},
        },
        LockWhileRunning::UNLOCKED,
        Path::PATH2
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(MODE);
        PA_ADD_OPTION(PATH0);
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    }
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(PLATFORM_RESET);
        PA_ADD_OPTION(NAVIGATE_TO_PLATFORM);
    }
    PA_ADD_OPTION(NOTIFICATIONS);
}





bool ShinyHuntAreaZeroPlatform::run_traversal(BotBaseContext& context){
    const ProgramInfo& info = m_env->program_info();
    ConsoleHandle& console = m_env->console;

    if (m_pending_save){
        save_game_from_overworld(info, console, context);
        m_pending_save = false;
        m_last_save = SavedLocation::AREA_ZERO;
    }

    WallClock start = current_time();

    size_t kills, encounters;
    std::chrono::minutes window_minutes(PLATFORM_RESET.WINDOW_IN_MINUTES);
    WallDuration window = m_time_tracker->last_window_in_realtime(start, window_minutes);
    std::chrono::seconds window_seconds = std::chrono::duration_cast<Seconds>(window);

    bool enough_time = m_encounter_tracker->get_encounters_in_window(
        kills, encounters, window_seconds
    );
    console.log(
        "Starting Traversal Iteration: " + tostr_u_commas(m_iterations) +
        "\n    Time Window (Seconds): " + std::to_string(window_seconds.count()) +
        "\n    Kills: " + std::to_string(kills) +
        "\n    Encounters: " + std::to_string(encounters)
    );

    do{
        if (!PLATFORM_RESET.enabled()){
            console.log("Platform Reset: Disabled", COLOR_ORANGE);
            break;
        }
        if (!enough_time || window_seconds.count() == 0){
            console.log("Platform Reset: Not enough time has elapsed.", COLOR_ORANGE);
            break;
        }
        if (kills >= PLATFORM_RESET.KILLS_IN_WINDOW){
            console.log("Platform Reset: Enough kills in window.", COLOR_ORANGE);
            break;
        }
        if (encounters >= PLATFORM_RESET.ENCOUNTERS_IN_WINDOW){
            console.log("Platform Reset: Enough encounters in window.", COLOR_ORANGE);
            break;
        }

        console.log("Conditions met for platform reset.");
        m_state = State::LEAVE_AND_RETURN;
        return false;
    }while (false);


    try{
        switch (PATH0){
        case Path::PATH0:
            area_zero_platform_run_path0(*m_env, console, context, *m_encounter_tracker, m_iterations);
            break;
        case Path::PATH1:
            area_zero_platform_run_path1(*m_env, console, context, *m_encounter_tracker, m_iterations);
            break;
        case Path::PATH2:
            area_zero_platform_run_path2(*m_env, console, context, *m_encounter_tracker, m_iterations);
            break;
        }
        m_iterations++;
    }catch (...){
        m_time_tracker->add_block(start, current_time());
        throw;
    }

    m_time_tracker->add_block(start, current_time());

    return true;
}



void ShinyHuntAreaZeroPlatform::run_state(BotBaseContext& context){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
    const ProgramInfo& info = m_env->program_info();
    ConsoleHandle& console = m_env->console;

    send_program_notification(
        *m_env, NOTIFICATION_STATUS_UPDATE,
        Color(0),
        "Program Status",
        {}, m_encounter_tracker->encounter_frequencies().dump_sorted_map("")
    );

#if 0
    WallClock now = current_time();
    if (m_last_sandwich + std::chrono::minutes() < now){
        m_state = State::RESET_SANDWICH;
    }
#endif

    State recovery_state = State::LEAVE_AND_RETURN;
    try{
        switch (m_state){
        case State::TRAVERSAL:{
            //  If we error out, recover using LEAVE_AND_RETURN.
            recovery_state = State::LEAVE_AND_RETURN;

            //  TODO: Wrap this with the sandwich timer cancel.
            if (run_traversal(context)){
                break;
            }else{
                return;
            }
        }
        case State::INSIDE_GATE_AND_RETURN:
            console.log("Going from inside gate to platform...");

            //  If we error out, recover using LEAVE_AND_RETURN.
            recovery_state = State::LEAVE_AND_RETURN;

            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);
            m_encounter_tracker->reset_rate_tracker_start_time();
//            m_last_platform_reset = now;

            break;

        case State::LEAVE_AND_RETURN:
            console.log("Leaving and returning to platform...");

            //  If we error out, return to this state.
            recovery_state = State::LEAVE_AND_RETURN;

            return_to_inside_zero_gate(info, console, context);
            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);

            stats.m_platform_resets++;
            m_encounter_tracker->reset_rate_tracker_start_time();
//            m_last_platform_reset = now;
            m_env->update_stats();

            break;

        case State::RESET_AND_RETURN:
            console.log("Resetting game and returning to platform...");

            //  If we error out, return to this state.
            recovery_state = State::RESET_AND_RETURN;

//            m_last_platform_reset = current_time();
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_RCLICK, 20, 105);

            switch (m_last_save){
            case SavedLocation::NONE:
                throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Cannot reset with no previous save.");
            case SavedLocation::ZERO_GATE_FLY_SPOT:
                return_to_inside_zero_gate(info, console, context);
                inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);
                break;
            case SavedLocation::AREA_ZERO:
                //  No further action needed as you're already on the platform.
                break;
            }

            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);

            stats.m_game_resets++;
            m_encounter_tracker->reset_rate_tracker_start_time();
//            m_last_platform_reset = now;
            m_env->update_stats();

            break;

        case State::RESET_SANDWICH:
            console.log("Resetting sandwich...");

            throw InternalProgramError(&console.logger(), PA_CURRENT_FUNCTION, "Sandwiches have not been implemented yet.");

//            break;
        }

        //  No problems. Go back to traversals.
        m_state = State::TRAVERSAL;
        m_consecutive_failures = 0;

    }catch (OperationFailedException& e){
        stats.m_errors++;
        m_env->update_stats();
        m_consecutive_failures++;
        m_state = recovery_state;
        e.send_notification(*m_env, NOTIFICATION_ERROR_RECOVERABLE);
        if (m_consecutive_failures >= 3){
            throw FatalProgramException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Failed 3 times consecutively."
            );
        }
    }
}

void ShinyHuntAreaZeroPlatform::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    m_env = &env;

    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    assert_16_9_720p_min(env.logger(), env.console);

//    pbf_press_button(context, BUTTON_RCLICK, 20, 105);

    m_iterations = 0;

    DiscontiguousTimeTracker time_tracker;
    m_time_tracker = &time_tracker;

    LetsGoEncounterBotTracker encounter_tracker(
        env, env.console, context,
        stats,
        LANGUAGE
    );
    m_encounter_tracker = &encounter_tracker;

    switch (MODE){
    case Mode::START_ON_PLATFORM:
        m_state = State::TRAVERSAL;
        break;
    case Mode::START_IN_ZERO_GATE:
        m_state = State::INSIDE_GATE_AND_RETURN;
        break;
    case Mode::MAKE_SANDWICH:
        throw UserSetupError(env.logger(), "Sandwich mode has not been implemented yet.");
    }

    m_pending_save = false;
    m_last_save = SavedLocation::NONE;
    m_last_sandwich = WallClock::min();

    //  This is the outer-most program loop that wraps all logic with the
    //  battle menu detector. If at any time you detect a battle menu, you break
    //  all the way out here to handle the encounter. This is needed because you
    //  can get attacked at almost any time while in Area Zero.
    m_consecutive_failures = 0;
    while (true){
        env.console.log("Starting encounter loop...", COLOR_PURPLE);
        EncounterWatcher encounter_watcher(env.console, COLOR_RED);
        run_until(
            env.console, context,
            [&](BotBaseContext& context){
                //  Inner program loop that runs the state machine.
                while (true){
                    run_state(context);
                }
            },
            {
                static_cast<VisualInferenceCallback&>(encounter_watcher),
                static_cast<AudioInferenceCallback&>(encounter_watcher),
            }
        );
        encounter_watcher.throw_if_no_sound();

        env.console.log("Detected battle.", COLOR_PURPLE);
        try{
            bool should_save = encounter_tracker.process_battle(encounter_watcher, ENCOUNTER_BOT_OPTIONS);
            if (should_save){
                m_pending_save = should_save;
            }
        }catch (ProgramFinishedException&){
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            throw;
        }
    }

//    GO_HOME_WHEN_DONE.run_end_of_program(context);
//    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
