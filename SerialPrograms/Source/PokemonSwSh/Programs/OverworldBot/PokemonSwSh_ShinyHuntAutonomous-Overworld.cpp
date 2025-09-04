/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_OverworldMovement.h"
#include "PokemonSwSh_OverworldTrajectory.h"
#include "PokemonSwSh_OverworldTrigger.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousOverworld_Descriptor::ShinyHuntAutonomousOverworld_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousOverworld",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Overworld",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Overworld.md",
        "Automatically shiny hunt overworld " + STRING_POKEMON + " with video feedback.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ShinyHuntAutonomousOverworld_Descriptor::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_resets(m_stats["Resets"])
    {
        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
        m_aliases["Unexpected Battles"] = "Errors";
    }
    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousOverworld_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ShinyHuntAutonomousOverworld::ShinyHuntAutonomousOverworld()
    : GO_HOME_WHEN_DONE(false)
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 0, 11
    )
    , MARK_OFFSET(
        "<b>Mark Offset:</b><br>Aim this far below the bottom of the exclamation/question mark. 1.0 is the height of the mark. "
        "Increase this value when the " + STRING_POKEMON + " are large.",
        LockMode::LOCK_WHILE_RUNNING,
        0.5, 0, 20
    )
    , MARK_PRIORITY(
        "<b>Mark Priority:</b><br>Favor exclamation marks or question marks?",
        {
            {MarkPriority::EXCLAMATION_ONLY,        "exclamation-only",         "Exclamation Marks Only (Ignore Question Marks)"},
            {MarkPriority::PRIORITIZE_EXCLAMATION,  "prioritize-exclamation",   "Prioritize Exclamation Marks"},
            {MarkPriority::NO_PREFERENCE,           "no-preference",            "No Preference"},
            {MarkPriority::PRIORITIZE_QUESTION,     "prioritize-question",      "Prioritize Question Marks"},
            {MarkPriority::QUESTION_ONLY,           "question-only",            "Question Marks Only (Ignore Exclamation Marks)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        MarkPriority::PRIORITIZE_EXCLAMATION
    )
    , TRIGGER_METHOD(
        "<b>Trigger Method:</b><br>How to trigger an overworld reaction mark?",
        {
            {TriggerMethod::WhistleOnly,            "whistle-only",         "Whistle Only"},
            {TriggerMethod::Whistle3Circle1,        "whistle3-circle1",     "Whistle 3 times, then circle once."},
            {TriggerMethod::Circle3Whistle3,        "circle3-whistle3",     "Circle 3 times, then whistle 3 times."},
            {TriggerMethod::CircleOnly,             "circle-only",          "Circle Only"},
            {TriggerMethod::Horizontal,             "horizontal",           "Horizontal Line Only"},
            {TriggerMethod::Whistle3Horizontal1,    "whistle3-horizontal1", "Whistle 3 times, then do horizontal line once."},
            {TriggerMethod::Horizontal3Whistle3,    "horizontal3-whistle3", "Do horizontal line 3 times, then whistle 3 times."},
            {TriggerMethod::Vertical,               "vertical",             "Vertical Line Only"},
            {TriggerMethod::Whistle3Vertical1,      "whistle3-vertical1",   "Whistle 3 times, then do vertical line once."},
            {TriggerMethod::Vertical3Whistle3,      "vertical3-whistle3",   "Do vertical line 3 times, then whistle 3 times."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TriggerMethod::Whistle3Circle1
    )
    , MAX_MOVE_DURATION0(
        "<b>Maximum Move Duration:</b><br>Do not move in the same direction for more than this long."
        " If you set this too high, you may wander too far from the grassy area.",
        LockMode::LOCK_WHILE_RUNNING,
        "1600 ms"
    )
    , MAX_TARGET_ALPHA(
        "<b>Max Target Alpha:</b><br>Ignore all targets with alpha larger than this. Set to zero to ignore all marks.",
        LockMode::LOCK_WHILE_RUNNING,
        70000, 0
    )
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , WATCHDOG_TIMER0(
        "<b>Watchdog Timer:</b><br>Reset the game if you go this long without any encounters.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
    , EXIT_BATTLE_TIMEOUT0(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "10 s"
    )
    , TARGET_CIRCLING(
        "<b>Target Circling:</b><br>After moving towards a " + STRING_POKEMON + ", make a circle."
        " This increases the chance of encountering the " + STRING_POKEMON + " if it has moved or if the trajectory missed.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(MARK_OFFSET);
    PA_ADD_OPTION(MARK_PRIORITY);
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MAX_MOVE_DURATION0);
    PA_ADD_OPTION(MAX_TARGET_ALPHA);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(WATCHDOG_TIMER0);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
    PA_ADD_OPTION(TARGET_CIRCLING);
}




bool ShinyHuntAutonomousOverworld::find_encounter(
    VideoStream& stream, ProControllerContext& context,
    ShinyHuntAutonomousOverworld_Descriptor::Stats& stats,
    WallClock expiration
) const{
    OverlayBoxScope self(
        stream.overlay(),
        {
            OverworldTargetTracker::OVERWORLD_CENTER_X - 0.02,
            OverworldTargetTracker::OVERWORLD_CENTER_Y - 0.05,
            0.04,
            0.1
        },
        COLOR_CYAN
    );

    OverworldTargetTracker target_tracker(
        stream.logger(), stream.overlay(),
        std::chrono::milliseconds(1000),
        MARK_OFFSET,
        MARK_PRIORITY,
        MAX_TARGET_ALPHA
    );

    std::unique_ptr<OverworldTrigger> trigger;
    switch (TRIGGER_METHOD){
    case TriggerMethod::WhistleOnly:
        trigger.reset(new OverworldTrigger_Whistle(target_tracker));
        break;
    case TriggerMethod::Whistle3Circle1:
        trigger.reset(new OverworldTrigger_WhistleCircle(target_tracker, true, 3, 1));
        break;
    case TriggerMethod::Circle3Whistle3:
        trigger.reset(new OverworldTrigger_WhistleCircle(target_tracker, false, 3, 3));
        break;
    case TriggerMethod::CircleOnly:
        trigger.reset(new OverworldTrigger_WhistleCircle(target_tracker, false, 0, 1));
        break;
    case TriggerMethod::Horizontal:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(target_tracker, false, 0, 1));
        break;
    case TriggerMethod::Whistle3Horizontal1:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(target_tracker, true, 3, 1));
        break;
    case TriggerMethod::Horizontal3Whistle3:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(target_tracker, false, 3, 3));
        break;
    case TriggerMethod::Vertical:
        trigger.reset(new OverworldTrigger_WhistleVertical(target_tracker, false, 0, 1));
        break;
    case TriggerMethod::Whistle3Vertical1:
        trigger.reset(new OverworldTrigger_WhistleVertical(target_tracker, true, 3, 1));
        break;
    case TriggerMethod::Vertical3Whistle3:
        trigger.reset(new OverworldTrigger_WhistleVertical(target_tracker, false, 3, 3));
        break;
    }

    while (true){
        //  Time expired.
        if (current_time() > expiration){
            return false;
        }

        target_tracker.clear_detections();

        //  Run trigger.
        {
            StandardBattleMenuWatcher battle_menu_detector(false);
            StartBattleWatcher start_battle_detector;

            int result = run_until<ProControllerContext>(
                stream, context,
                [&](ProControllerContext& context){
                    trigger->run(context);
                },
                {
                    {battle_menu_detector},
                    {start_battle_detector},
                    {target_tracker},
                }
            );

            switch (result){
            case 0:
                stream.log("Unexpected Battle.", COLOR_RED);
                run_away(stream, context, EXIT_BATTLE_TIMEOUT0);
                return false;
            case 1:
                stream.log("Battle started!");
                return true;
            }
        }

        std::pair<double, OverworldTarget> target = target_tracker.best_target();
        target_tracker.clear_detections();

//        env.log("target: " + std::to_string(target.first));

        if (target.first < 0){
            stream.log("No targets found.", COLOR_ORANGE);
            continue;
        }
        if (target.first > MAX_TARGET_ALPHA){
            stream.log(
                std::string("Target too Weak: ") +
                (target.second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
                " at [" +
                tostr_default(target.second.delta_x) + " , " +
                tostr_default(-target.second.delta_y) + "], alpha = " +
                tostr_default(target.first),
                COLOR_ORANGE
            );
            continue;
        }

        if (charge_at_target(stream, context, target)){
            return true;
        }
    }
}


bool ShinyHuntAutonomousOverworld::charge_at_target(
    VideoStream& stream, ProControllerContext& context,
    const std::pair<double, OverworldTarget>& target
) const{
    OverlayBoxScope target_box(stream.overlay(), target.second.box, COLOR_YELLOW);
    stream.log(
        std::string("Best Target: ") +
        (target.second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
        " at [" +
        tostr_default(target.second.delta_x) + " , " +
        tostr_default(-target.second.delta_y) + "], alpha = " +
        tostr_default(target.first),
        COLOR_PURPLE
    );

    const Trajectory& trajectory = target.second.trajectory;
    double angle = std::atan2(
        (double)trajectory.joystick_y - 128,
        (double)trajectory.joystick_x - 128
    ) * 57.295779513082320877;
    stream.log(
        "Trajectory: Distance = " + std::to_string(trajectory.distance_in_ticks) +
        ", Direction = " + tostr_default(-angle) + " degrees"
    );

    Milliseconds duration = (trajectory.distance_in_ticks + 16) * 8ms;
    duration = std::min<Milliseconds>(duration, MAX_MOVE_DURATION0);


    StandardBattleMenuWatcher battle_menu_detector(false);
    StartBattleWatcher start_battle_detector;
    OverworldTargetTracker target_tracker(
        stream.logger(), stream.overlay(),
        std::chrono::milliseconds(1000),
        MARK_OFFSET,
        MARK_PRIORITY,
        MAX_TARGET_ALPHA
    );

    int result = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            //  Move to target.
            pbf_move_left_joystick(
                context,
                trajectory.joystick_x,
                trajectory.joystick_y,
                duration, 0ms
            );

            //  Circle Maneuver
            if (TARGET_CIRCLING){
                if (trajectory.joystick_y < 64 &&
                    64 <= trajectory.joystick_x && trajectory.joystick_x <= 192
                ){
                    move_in_circle_up(context, trajectory.joystick_x > 128);
                }else{
                    move_in_circle_down(context, trajectory.joystick_x <= 128);
                }
            }
        },
        {
            {battle_menu_detector},
            {start_battle_detector},
            {target_tracker},
        }
    );

    switch (result){
    case 0:
        stream.log("Unexpected Battle.", COLOR_RED);
        return true;
    case 1:
        stream.log("Battle started!");
        return true;
    default:
        return false;
    }
}






void ShinyHuntAutonomousOverworld::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    srand((unsigned)time(nullptr));

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }
    pbf_move_right_joystick(context, 128, 255, TICKS_PER_SECOND, 0);

    WallDuration TIMEOUT = WATCHDOG_TIMER0;
    WallDuration PERIOD = std::chrono::hours(TIME_ROLLBACK_HOURS);
    WallClock last_touch = current_time();

    ShinyHuntAutonomousOverworld_Descriptor::Stats& stats = env.current_stats<ShinyHuntAutonomousOverworld_Descriptor::Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    //  Encounter Loop
    auto last = current_time();
    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && current_time() - last_touch >= PERIOD){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
            rollback_hours_from_home(env.console, context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        auto now = current_time();
        if (now - last > TIMEOUT){
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
            reset_game_from_home_with_inference(
                env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
            stats.m_resets++;
            last = current_time();
            continue;
        }

        context.wait_for_all_requests();

        bool battle = find_encounter(env.console, context, stats, last + TIMEOUT);
        if (!battle){
            continue;
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT0);
        if (stop){
            break;
        }

        last = current_time();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}

