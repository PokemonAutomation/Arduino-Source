/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/OCR/Filtering.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_OverworldMovement.h"
#include "PokemonSwSh_OverworldTargetTracker.h"
#include "PokemonSwSh_OverworldTrajectory.h"
#include "PokemonSwSh_OverworldTrigger.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousOverworld_Descriptor::ShinyHuntAutonomousOverworld_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousOverworld",
        "Shiny Hunt Autonomous - Overworld",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-Overworld",
        "Automatically shiny hunt overworld " + STRING_POKEMON + " with video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousOverworld::ShinyHuntAutonomousOverworld(const ShinyHuntAutonomousOverworld_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(m_name_reader)
    , MARK_OFFSET(
        "<b>Mark Offset:</b><br>Aim this far below the bottom of the exclamation/question mark. 1.0 is the height of the mark. "
        "Increase this value when the " + STRING_POKEMON + " are large.",
        0.5, 0, 20
    )
    , MARK_PRIORITY(
        "<b>Mark Priority:</b><br>Favor exclamation marks or question marks?",
        MARK_PRIORITY_STRINGS, 1
    )
    , TRIGGER_METHOD(
        "<b>Trigger Method:</b><br>How to trigger an overworld reaction mark?",
        {
            "Whistle Only",
            "Whistle 3 times, then circle once.",
            "Circle 3 times, then whistle 3 times.",
            "Circle Only",
            "Horizontal Line Only",
            "Whistle 3 times, then do horizontal line once.",
            "Do horizontal line 3 times, then whistle 3 times.",
            "Vertical Line Only",
            "Whistle 3 times, then do vertical line once.",
            "Do vertical line 3 times, then whistle 3 times.",
        }, 1
    )
    , MAX_MOVE_DURATION(
        "<b>Maximum Move Duration:</b><br>Do not move in the same direction for more than this long."
        " If you set this too high, you may wander too far from the grassy area.",
        "200"
    )
    , MAX_TARGET_ALPHA(
        "<b>Max Target Alpha:</b><br>Ignore all targets with alpha larger than this. Set to zero to ignore all marks.",
        70000, 0
    )
    , FILTER(true, true)
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , WATCHDOG_TIMER(
        "<b>Watchdog Timer:</b><br>Reset the game if you go this long without any encounters.",
        "60 * TICKS_PER_SECOND"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
    , TARGET_CIRCLING(
        "<b>Target Circling:</b><br>After moving towards a " + STRING_POKEMON + ", make a circle."
        " This increases the chance of encountering the " + STRING_POKEMON + " if it has moved or if the trajectory missed.",
        true
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");

    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&MARK_OFFSET, "MARK_OFFSET");
    m_options.emplace_back(&MARK_PRIORITY, "MARK_PRIORITY");
    m_options.emplace_back(&TRIGGER_METHOD, "TRIGGER_METHOD");
    m_options.emplace_back(&MAX_MOVE_DURATION, "MAX_MOVE_DURATION");
    m_options.emplace_back(&MAX_TARGET_ALPHA, "MAX_TARGET_ALPHA");
    m_options.emplace_back(&FILTER, "FILTER");

    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    m_options.emplace_back(&NOTIFICATION_LEVEL, "NOTIFICATION_LEVEL");

    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&WATCHDOG_TIMER, "WATCHDOG_TIMER");
    m_options.emplace_back(&EXIT_BATTLE_TIMEOUT, "EXIT_BATTLE_TIMEOUT");
    m_options.emplace_back(&TARGET_CIRCLING, "ENABLE_CIRCLING");
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
    }
}



struct ShinyHuntAutonomousOverworld::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_resets(m_stats["Resets"])
    {
        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
        m_aliases["Unexpected Battles"] = "Errors";
    }
    uint64_t& m_resets;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousOverworld::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



bool ShinyHuntAutonomousOverworld::find_encounter(
    SingleSwitchProgramEnvironment& env,
    Stats& stats,
    std::chrono::system_clock::time_point expiration
) const{
    InferenceBoxScope self(
        env.console, Qt::cyan,
        OverworldTargetTracker::OVERWORLD_CENTER_X - 0.02,
        OverworldTargetTracker::OVERWORLD_CENTER_Y - 0.05,
        0.04, 0.1
    );

    InterruptableCommandSession commands(env.console);

    StandardBattleMenuDetector battle_menu_detector(env.console);
    battle_menu_detector.register_command_stop(commands);

    StartBattleDetector start_battle_detector(env.console);
    start_battle_detector.register_command_stop(commands);

    OverworldTargetTracker target_tracker(
        env.logger(), env.console,
        std::chrono::milliseconds(1000),
        MARK_OFFSET,
        (MarkPriority)(size_t)MARK_PRIORITY,
        MAX_TARGET_ALPHA
    );
    target_tracker.register_command_stop(commands);

    std::unique_ptr<OverworldTrigger> trigger;
    switch ((size_t)TRIGGER_METHOD){
    case 0:
        trigger.reset(new OverworldTrigger_Whistle(env, commands, target_tracker));
        break;
    case 1:
        trigger.reset(new OverworldTrigger_WhistleCircle(env, commands, target_tracker, true, 3, 1));
        break;
    case 2:
        trigger.reset(new OverworldTrigger_WhistleCircle(env, commands, target_tracker, false, 3, 3));
        break;
    case 3:
        trigger.reset(new OverworldTrigger_WhistleCircle(env, commands, target_tracker, false, 0, 1));
        break;
    case 4:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(env, commands, target_tracker, false, 0, 1));
        break;
    case 5:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(env, commands, target_tracker, true, 3, 1));
        break;
    case 6:
        trigger.reset(new OverworldTrigger_WhistleHorizontal(env, commands, target_tracker, false, 3, 3));
        break;
    case 7:
        trigger.reset(new OverworldTrigger_WhistleVertical(env, commands, target_tracker, false, 0, 1));
        break;
    case 8:
        trigger.reset(new OverworldTrigger_WhistleVertical(env, commands, target_tracker, true, 3, 1));
        break;
    case 9:
        trigger.reset(new OverworldTrigger_WhistleVertical(env, commands, target_tracker, false, 3, 3));
        break;
    }

    size_t loops = 0;
    while (true){
        loops++;

        //  Time expired.
        if (std::chrono::system_clock::now() > expiration){
            return false;
        }

        if (battle_menu_detector.triggered()){
            env.log("Unexpected Battle.", "red");
            stats.add_error();
            run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
            return false;
        }
        if (start_battle_detector.triggered()){
            env.log("Battle started!");
            return true;
        }

        std::pair<double, OverworldTarget> target = target_tracker.best_target();

        target_tracker.clear_detections();
        AsyncVisualInferenceSession inference(env, env.console);
        inference += battle_menu_detector;
        inference += start_battle_detector;
        inference += target_tracker;

        //  No target found.
        if (target.first < 0 || target.first > MAX_TARGET_ALPHA){
            if (target.first < 0){
                if (loops > 1){
                    env.log("No targets found.", "orange");
//                    pbf_press_button(env.console, BUTTON_B, 5, 0);
                }
            }else{
                env.log(
                    QString("Target too Weak: ") +
                    (target.second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
                    " at [" +
                    QString::number(target.second.delta_x) + " , " +
                    QString::number(-target.second.delta_y) + "], alpha = " +
                    QString::number(target.first),
                    "orange"
                );
            }
            trigger->run();
            continue;
        }


        //  Target Found
        target.second.box.color = Qt::yellow;
        InferenceBoxScope target_box(env.console, target.second.box);
        env.log(
            QString("Best Target: ") +
            (target.second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
            " at [" +
            QString::number(target.second.delta_x) + " , " +
            QString::number(-target.second.delta_y) + "], alpha = " +
            QString::number(target.first),
            "purple"
        );

        const Trajectory& trajectory = target.second.trajectory;
        double angle = std::atan2(
            (double)trajectory.joystick_y - 128,
            (double)trajectory.joystick_x - 128
        ) * 57.295779513082320877;
        env.log(
            "Trajectory: Distance = " + QString::number(trajectory.distance_in_ticks) +
            ", Direction = " + QString::number(-angle) + " degrees"
        );

        int duration = trajectory.distance_in_ticks + 16;
        if (duration > (int)MAX_MOVE_DURATION){
            duration = MAX_MOVE_DURATION;
        }

        //  Move to target.
        pbf_move_left_joystick(
            env.console,
            trajectory.joystick_x,
            trajectory.joystick_y,
            (uint16_t)duration, 0
        );

        //  Circle Maneuver
        if (TARGET_CIRCLING){
            if (
                trajectory.joystick_y < 64 &&
                64 <= trajectory.joystick_x && trajectory.joystick_x <= 192
            ){
                move_in_circle_up(env.console, trajectory.joystick_x > 128);
            }else{
                move_in_circle_down(env.console, trajectory.joystick_x <= 128);
            }
        }
        env.console.botbase().wait_for_all_requests();
        target_tracker.clear_detections();
    }
}

void ShinyHuntAutonomousOverworld::program(SingleSwitchProgramEnvironment& env){
    srand(time(nullptr));

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }
    pbf_move_right_joystick(env.console, 128, 255, TICKS_PER_SECOND, 0);

    const std::chrono::milliseconds TIMEOUT((uint64_t)WATCHDOG_TIMER * 1000 / TICKS_PER_SECOND);
    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        &m_name_reader, LANGUAGE,
        stats,
        FILTER,
        VIDEO_ON_SHINY,
        NOTIFICATION_LEVEL
    );

    //  Encounter Loop
    auto last = std::chrono::system_clock::now();
    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

//        cout << "TOLERATE_SYSTEM_UPDATE_MENU_FAST = " << TOLERATE_SYSTEM_UPDATE_MENU_FAST << endl;

        auto now = std::chrono::system_clock::now();
        if (now - last > TIMEOUT){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
            stats.m_resets++;
            last = std::chrono::system_clock::now();
            continue;
        }

        env.console.botbase().wait_for_all_requests();

        bool battle = find_encounter(env, stats, last + TIMEOUT);
        if (!battle){
            continue;
        }

        //  Detect shiny.
        ShinyType shininess = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;

        bool stop = handler.handle_standard_encounter_runaway(shininess, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }

        last = std::chrono::system_clock::now();
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

