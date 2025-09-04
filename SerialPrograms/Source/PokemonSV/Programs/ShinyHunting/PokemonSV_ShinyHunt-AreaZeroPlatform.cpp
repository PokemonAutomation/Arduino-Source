/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_AreaZero.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
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
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntAreaZeroPlatform_Descriptor::Stats : public LetsGoEncounterBotStats{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_autoheals(m_stats["Auto Heals"])
        , m_platform_resets(m_stats["Platform Resets"])
        , m_game_resets(m_stats["Game Resets"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 2, {"Sandwiches", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 3, {"Auto Heals", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 4, {"Platform Resets", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 5, {"Game Resets", ALWAYS_HIDDEN});
        m_display_order.insert(m_display_order.begin() + 6, {"Errors", HIDDEN_IF_ZERO});
    }
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_autoheals;
    std::atomic<uint64_t>& m_platform_resets;
    std::atomic<uint64_t>& m_game_resets;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> ShinyHuntAreaZeroPlatform_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




ShinyHuntAreaZeroPlatform::~ShinyHuntAreaZeroPlatform(){
    MODE.remove_listener(*this);
}

ShinyHuntAreaZeroPlatform::ShinyHuntAreaZeroPlatform()
    : LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , MODE(
        "<b>Mode:</b><br>"
        "If starting on the platform, you should stand near the center of the platform facing any direction.<br>"
        "If starting at the Zero Gate fly spot, you should be at the fly spot as if you just flew there."
        "<br>If making a sandwich, you should be at the Zero Gate fly spot as if you just flew there.",
        {
            {Mode::START_ON_PLATFORM,   "platform", "Start on platform."},
            {Mode::START_AT_ZERO_GATE_FLY_SPOT,  "zerogate", "Start at Zero Gate fly spot."},
            {Mode::MAKE_SANDWICH,       "sandwich", "Make a sandwich."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::START_ON_PLATFORM
    )
    , FLYING_UNLOCKED(
        "<b>Flying Unlocked:</b><br>Check this box if you have unlocked your ride's ability to fly after completing the Indigo Disk DLC.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , PATH0(
        "<b>Path:</b><br>Traversal path on the platform to trigger encounters.",
        {
            {Path::PATH0, "path0", "Path 0"},
            {Path::PATH1, "path1", "Path 1"},
            {Path::PATH2, "path2", "Path 2"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        Path::PATH2
    )
    , SANDWICH_RESET_IN_MINUTES(
        "<b>Sandwich Reset Time (in minutes):</b><br>The time to reset game to make a new sandwich.",
        LockMode::UNLOCK_WHILE_RUNNING,
        35
    )
    , SANDWICH_OPTIONS(
        "Sandwich Options",
        &LANGUAGE,
        BaseRecipe::paradox,
        false,
        GroupOption::EnableMode::ALWAYS_ENABLED
    )
    , GO_HOME_WHEN_DONE(true)
    , AUTO_HEAL_PERCENT(
        "<b>Auto-Heal %</b><br>Auto-heal if your HP drops below this percentage.",
        LockMode::UNLOCK_WHILE_RUNNING,
        75, 0, 100
    )
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
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(FLYING_UNLOCKED);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(PATH0);
    }
    PA_ADD_OPTION(SANDWICH_RESET_IN_MINUTES);
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(AUTO_HEAL_PERCENT);
    PA_ADD_OPTION(PLATFORM_RESET);
    PA_ADD_OPTION(NAVIGATE_TO_PLATFORM);
    PA_ADD_OPTION(NOTIFICATIONS);

    ShinyHuntAreaZeroPlatform::on_config_value_changed(this);

    MODE.add_listener(*this);
}

std::string ShinyHuntAreaZeroPlatform::check_validity() const{
    std::string error = SingleSwitchProgramInstance::check_validity();
    if (!error.empty()){
        return error;
    }
    if (LANGUAGE == Language::None && MODE == Mode::MAKE_SANDWICH){
        return "Sandwich mode requires selecting a language.";
    }
    return "";
}
void ShinyHuntAreaZeroPlatform::on_config_value_changed(void* object){
    ConfigOptionState state = MODE == Mode::MAKE_SANDWICH
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN;
    SANDWICH_RESET_IN_MINUTES.set_visibility(state);
    SANDWICH_OPTIONS.set_visibility(state);
}












bool ShinyHuntAreaZeroPlatform::run_traversal(ProControllerContext& context){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    const ProgramInfo& info = m_env->program_info();
    VideoStream& stream = m_env->console;

//    if (m_pending_save){
//        save_game_from_overworld(info, console, context);
//        m_pending_save = false;
//        m_last_save = SavedLocation::AREA_ZERO;
//    }

    double hp = m_sensors->lets_go_hp.last_known_value() * 100;
    if (0 < hp){
        stream.log("Last Known HP: " + tostr_default(hp) + "%", COLOR_BLUE);
    }else{
        stream.log("Last Known HP: ?", COLOR_RED);
    }
    if (0 < hp && hp < AUTO_HEAL_PERCENT){
        auto_heal_from_menu_or_overworld(info, stream, context, 0, true);
        stats.m_autoheals++;
        m_env->update_stats();
    }

    WallClock start = current_time();

    size_t kills = 0, encounters = 0;
    std::chrono::minutes window_minutes(PLATFORM_RESET.WINDOW_IN_MINUTES);
    WallDuration window = m_time_tracker->last_window_in_realtime(start, window_minutes);

    std::chrono::seconds window_seconds;
    bool enough_time;
    if (window == WallDuration::zero()){
//        stream.log("Debug Reset Timer: Window not initialized.", COLOR_RED);

        //  Not enough history.
        enough_time = false;
        window = start - m_encounter_tracker->encounter_rate_tracker_start_time();
        window_seconds = std::chrono::duration_cast<Seconds>(window);
        m_encounter_tracker->get_encounters_in_window(
            kills, encounters, window_seconds
        );
    }else{
//        stream.log("Debug Reset Timer: Window started.", COLOR_RED);

        window_seconds = std::chrono::duration_cast<Seconds>(window);
        enough_time = m_encounter_tracker->get_encounters_in_window(
            kills, encounters, window_seconds
        );
    }
    stream.log(
        "Starting Traversal Iteration: " + tostr_u_commas(m_iterations) +
        "\n    Time Window (Seconds): " + std::to_string(window_seconds.count()) +
        "\n    Kills: " + std::to_string(kills) +
        "\n    Encounters: " + std::to_string(encounters)
    );

    // Check we want to do a platform reset first:
    do{
        if (!PLATFORM_RESET.enabled()){
            stream.log("Platform Reset: Disabled", COLOR_ORANGE);
            break;
        }
        if (!enough_time){
            stream.log("Platform Reset: Not enough time has elapsed.", COLOR_ORANGE);
            break;
        }
        if (kills >= PLATFORM_RESET.KILLS_IN_WINDOW0){
            stream.log("Platform Reset: Enough kills in window.", COLOR_ORANGE);
            break;
        }
        if (encounters >= PLATFORM_RESET.ENCOUNTERS_IN_WINDOW){
            stream.log("Platform Reset: Enough encounters in window.", COLOR_ORANGE);
            break;
        }

        stream.log("Conditions met for platform reset.");
        m_pending_platform_reset = true;
//        m_state = State::LEAVE_AND_RETURN;
        return false;
    }while (false);

    // Send Let's Go pokemon to beat wild pokemon while moving on the platform following one path.
    // It tracks the kill chain by sound detection from `m_encounter_tracker`.
    try{
        switch (PATH0){
        case Path::PATH0:
            area_zero_platform_run_path0(*m_env, stream, context, *m_encounter_tracker, m_iterations);
            break;
        case Path::PATH1:
            area_zero_platform_run_path1(*m_env, stream, context, *m_encounter_tracker, m_iterations);
            break;
        case Path::PATH2:
            area_zero_platform_run_path2(*m_env, stream, context, *m_encounter_tracker, m_iterations);
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


struct ResetException{};


void ShinyHuntAreaZeroPlatform::set_flags(SingleSwitchProgramEnvironment& env){
//    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
//    const ProgramInfo& info = m_env->program_info();
    VideoStream& stream = m_env->console;

    send_program_notification(
        *m_env, NOTIFICATION_STATUS_UPDATE,
        Color(0),
        "Program Status",
        {}, m_encounter_tracker->encounter_frequencies().dump_sorted_map("")
    );

    WallClock now = current_time();
    if (MODE == Mode::MAKE_SANDWICH &&
        m_last_sandwich + std::chrono::minutes(SANDWICH_RESET_IN_MINUTES) < now
    ){
        stream.log("Enough time has elapsed. Time to reset sandwich...");
        m_pending_sandwich = true;
    }

    int64_t seconds_on_sandwich = std::chrono::duration_cast<std::chrono::seconds>(now - m_last_sandwich).count();
    stream.log(
        std::string("State:\n") +
        "    Time on Sandwich: " + (m_last_sandwich == WallClock::min()
            ? "N/A"
            : std::to_string(seconds_on_sandwich)) + " seconds\n" +
        "    Pending Save: " + (m_pending_save ? "Yes" : "No") + "\n" +
        "    Pending Platform Reset: " + (m_pending_platform_reset ? "Yes" : "No") + "\n" +
        "    Pending Sandwich: " + (m_pending_sandwich ? "Yes" : "No") + "\n" +
        "    Reset after Sandwich: " + (m_reset_on_next_sandwich ? "Yes" : "No") + "\n"
    );

}
void ShinyHuntAreaZeroPlatform::run_state(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
    const ProgramInfo& info = m_env->program_info();
    VideoStream& stream = m_env->console;

    if (m_pending_save){
        stream.log("Executing: Pending Save...");
        if (m_current_location != Location::ZERO_GATE_FLY_SPOT && m_current_location != Location::AREA_ZERO){
            return_to_outside_zero_gate(info, stream, context);
            m_current_location = Location::ZERO_GATE_FLY_SPOT;
        }
        save_game_from_overworld(info, stream, context);
        m_saved_location = m_current_location;
        m_pending_save = false;
        return;
    }

    if (m_pending_sandwich){
        stream.log("Executing: Pending Sandwich...");

        //  If we need to reset, do so now.
        if (m_reset_on_next_sandwich){
            throw ResetException();
        }

        //  If we're not at Zero Gate, go there now.
        if (m_current_location != Location::ZERO_GATE_FLY_SPOT){
            return_to_outside_zero_gate(info, stream, context);
            m_current_location = Location::ZERO_GATE_FLY_SPOT;
            m_pending_platform_reset = false;
        }

        m_reset_on_next_sandwich = true;

        //  If we're not saved at Zero Gate, do it now.
        if (m_saved_location != Location::ZERO_GATE_FLY_SPOT){
            save_game_from_overworld(info, stream, context);
            m_saved_location = m_current_location;
        }

        picnic_at_zero_gate(info, stream, context);
        pbf_move_left_joystick(context, 128, 0, 70, 0);
        enter_sandwich_recipe_list(info, stream, context);
        make_sandwich_option(env, stream, context, SANDWICH_OPTIONS);

        stream.log("Sandwich Reset: Starting new sandwich timer...");
        m_last_sandwich = current_time();

        stats.m_sandwiches++;
        m_env->update_stats();

        leave_picnic(info, stream, context);
        return_to_inside_zero_gate_from_picnic(info, stream, context);
        inside_zero_gate_to_platform(info, stream, context, FLYING_UNLOCKED, NAVIGATE_TO_PLATFORM);
        m_current_location = Location::AREA_ZERO;

        m_pending_sandwich = false;

        m_encounter_tracker->reset_rate_tracker_start_time();
        m_consecutive_failures = 0;

        return;
    }

    if (m_pending_platform_reset){
        stream.log("Executing: Platform Reset");
        return_to_inside_zero_gate(info, stream, context);
        inside_zero_gate_to_platform(info, stream, context, FLYING_UNLOCKED, NAVIGATE_TO_PLATFORM);
        m_current_location = Location::AREA_ZERO;

        stats.m_platform_resets++;
        m_env->update_stats();

        m_pending_platform_reset = false;
        m_encounter_tracker->reset_rate_tracker_start_time();
        m_consecutive_failures = 0;
        return;
    }

    switch (m_current_location){
    case Location::UNKNOWN:
    case Location::ZERO_GATE_FLY_SPOT:
    case Location::TRAVELING_TO_PLATFORM:
        stream.log("Executing: Platform Reset (state-based)...");
        return_to_inside_zero_gate(info, stream, context);
        inside_zero_gate_to_platform(info, stream, context, FLYING_UNLOCKED, NAVIGATE_TO_PLATFORM);
        m_current_location = Location::AREA_ZERO;
        m_pending_platform_reset = false;
        m_encounter_tracker->reset_rate_tracker_start_time();
        m_consecutive_failures = 0;
        return;
    case Location::AREA_ZERO:
        stream.log("Executing: Traversal...");
        try{
            run_traversal(context);
        }catch (OperationFailedException&){
            m_pending_platform_reset = true;
            throw;
        }
//        m_encounter_tracker->reset_rate_tracker_start_time();
        m_consecutive_failures = 0;
        return;
    }
}
void ShinyHuntAreaZeroPlatform::set_flags_and_run_state(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    set_flags(env);

    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
//    const ProgramInfo& info = m_env->program_info();
    VideoStream& stream = m_env->console;

    try{
        run_state(env, context);
    }catch (OperationFailedException& e){
        stats.m_errors++;
        m_env->update_stats();
        m_consecutive_failures++;
        e.send_notification(*m_env, NOTIFICATION_ERROR_RECOVERABLE);
        if (m_consecutive_failures >= 3){
            throw_and_log<FatalProgramException>(
                stream.logger(),
                ErrorReport::SEND_ERROR_REPORT,
                "Failed 3 times consecutively."
            );
        }
    }
}







void ShinyHuntAreaZeroPlatform::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    m_env = &env;

    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    assert_16_9_720p_min(env.logger(), env.console);

    m_iterations = 0;

    OverworldSensors sensors(
        env.logger(), env.console, context
    );
    m_sensors = &sensors;

    OverworldBattleTracker battle_tracker(env.logger(), sensors);

    DiscontiguousTimeTracker time_tracker;
    m_time_tracker = &time_tracker;

    LetsGoEncounterBotTracker encounter_tracker(
        env, env.console,
        stats,
        sensors.lets_go_kill
    );
    m_encounter_tracker = &encounter_tracker;

    m_saved_location = Location::UNKNOWN;
    m_pending_save = false;
    m_pending_platform_reset = false;
    m_pending_sandwich = false;
    m_reset_on_next_sandwich = false;

    switch (MODE){
    case Mode::START_ON_PLATFORM:
        m_current_location = Location::AREA_ZERO;
//        m_state = State::TRAVERSAL;
        break;
    case Mode::START_AT_ZERO_GATE_FLY_SPOT:
        m_current_location = Location::ZERO_GATE_FLY_SPOT;
//        m_state = State::INSIDE_GATE_AND_RETURN;
        break;
    case Mode::MAKE_SANDWICH:
        m_current_location = Location::UNKNOWN;
        m_pending_save = false;
        m_pending_sandwich = true;
//        m_state = State::RESET_SANDWICH;
        break;
    }


//    m_pending_save = false;
//    m_last_save = SavedLocation::NONE;
    m_last_sandwich = WallClock::min();

    //  This is the outer-most program loop that wraps all logic with the
    //  battle menu detector. If at any time you detect a battle menu, you break
    //  all the way out here to handle the encounter. This is needed because you
    //  can get attacked at almost any time while in Area Zero.
    m_consecutive_failures = 0;
    while (true){
        try{
            env.console.log("Starting encounter loop...", COLOR_PURPLE);
            NormalBattleMenuWatcher battle_menu(COLOR_RED);
            run_until<ProControllerContext>(
                env.console, context,
                [&](ProControllerContext& context){
                    //  Inner program loop that runs the state machine.
                    while (true){
                        set_flags_and_run_state(env, context);
                    }
                },
                {battle_menu}
            );
            sensors.throw_if_no_sound();

            env.console.log("Detected battle.", COLOR_PURPLE);
            stats.m_encounters++;
            env.update_stats();
            encounter_tracker.encounter_rate_tracker().report_encounter();

            bool caught, should_save;
            process_battle(
                caught, should_save,
                env,
                ENCOUNTER_BOT_OPTIONS,
                env.console, context,
                battle_tracker,
                encounter_tracker.encounter_frequencies(),
                stats.m_shinies,
                LANGUAGE
            );

            m_pending_save |= should_save;
            if (caught){
                m_reset_on_next_sandwich = false;
            }
        }catch (ResetException&){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
            reset_game_from_home_zoom_out(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
            m_current_location = m_saved_location;
            stats.m_game_resets++;
            m_env->update_stats();
            m_pending_platform_reset = false;
            m_reset_on_next_sandwich = false;
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
