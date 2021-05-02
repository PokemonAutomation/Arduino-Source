/*  Shiny Hunt Autonomous - Overworld
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Programs/PokemonSwSh_OverworldTrajectory.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



ShinyHuntAutonomousOverworld::ShinyHuntAutonomousOverworld()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Overworld",
        "SerialPrograms/ShinyHuntAutonomous-Overworld.md",
        "Automatically shiny hunt overworld " + STRING_POKEMON + " with video feedback."
    )
    , GO_HOME_WHEN_DONE(
        "<b>Go Home when Done:</b><br>After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
        false
    )
    , MARK_OFFSET(
        "<b>Mark Offset:</b><br>Aim this far below the bottom of the exclamation/question mark. 1.0 is the height of the mark. "
        "Increase this value when the " + STRING_POKEMON + " are large.",
        0.5, 0, 20
    )
    , PRIORITIZE_EXCLAMATION_POINTS(
        "<b>Prioritize Exclamation Points:</b><br>Given multiple options, prefer those with exclamation points."
        " This prioritizes random grass encounters and " + STRING_POKEMON + " that flee.",
        true
    )
    , TARGET_CIRCLING(
        "<b>Target Circling:</b><br>After moving towards a " + STRING_POKEMON + ", make a circle."
        " This increases the chance of encountering the " + STRING_POKEMON + " if it has moved or if the trajectory missed.",
        true
    )
    , LOCAL_CIRCLING(
        "<b>Local Circling:</b><br>If nothing is found after this many whistles, run in a circle."
        " Set this to zero to disable this feature.",
        3, 0, 10
    )
    , MAX_MOVE_DURATION(
        "<b>Maximum Move Duration:</b><br>Do not move in the same direction for more than this long."
        " If you set this too high, you may wander too far from the grassy area.",
        "200"
    )
    , WATCHDOG_TIMER(
        "<b>Watchdog Timer:</b><br>Reset the game if you go this long without any encounters.",
        "120 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld.",
        "6 * TICKS_PER_SECOND"
    )
    , MAX_TARGET_ALPHA(
        "<b>Max Target Alpha:</b><br>Ignore all targets with alpha larger than this.",
        50000, 0
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
    , RUN_FROM_EVERYTHING(
        "<b>Run from Everything:</b><br>Run from everything - even if it is shiny. (For testing only.)",
        false
    )
{
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");
    m_options.emplace_back(&MARK_OFFSET, "MARK_OFFSET");
    m_options.emplace_back(&PRIORITIZE_EXCLAMATION_POINTS, "PRIORITIZE_EXCLAMATION_POINTS");
    m_options.emplace_back(&TARGET_CIRCLING, "ENABLE_CIRCLING");
    m_options.emplace_back(&LOCAL_CIRCLING, "LOCAL_CIRCLING");
    m_options.emplace_back(&MAX_MOVE_DURATION, "MAX_MOVE_DURATION");
    m_options.emplace_back(&WATCHDOG_TIMER, "WATCHDOG_TIMER");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    if (settings.developer_mode){
        m_options.emplace_back(&MAX_TARGET_ALPHA, "MAX_TARGET_ALPHA");
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
        m_options.emplace_back(&RUN_FROM_EVERYTHING, "RUN_FROM_EVERYTHING");
    }
}



struct ShinyHuntAutonomousOverworld::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_errors(m_stats["Errors"])
        , m_resets(m_stats["Resets"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Errors"));
        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
    }
    uint64_t& m_errors;
    uint64_t& m_resets;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousOverworld::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void ShinyHuntAutonomousOverworld::move_in_circle_up(
    SingleSwitchProgramEnvironment& env,
    bool counter_clockwise
) const{
//    cout << "up" << endl;
    if (counter_clockwise){
        pbf_move_left_joystick(env.console, 255, 128, 16, 0);
        pbf_move_left_joystick(env.console, 255, 0, 16, 0);
        pbf_move_left_joystick(env.console, 128, 0, 16, 0);
        pbf_move_left_joystick(env.console, 0, 0, 16, 0);
        pbf_move_left_joystick(env.console, 0, 128, 16, 0);
        pbf_move_left_joystick(env.console, 0, 255, 16, 0);
        pbf_move_left_joystick(env.console, 128, 255, 16, 0);
        pbf_move_left_joystick(env.console, 255, 255, 16, 0);
    }else{
        pbf_move_left_joystick(env.console, 0, 128, 16, 0);
        pbf_move_left_joystick(env.console, 0, 0, 16, 0);
        pbf_move_left_joystick(env.console, 128, 0, 16, 0);
        pbf_move_left_joystick(env.console, 255, 0, 16, 0);
        pbf_move_left_joystick(env.console, 255, 128, 16, 0);
        pbf_move_left_joystick(env.console, 255, 255, 16, 0);
        pbf_move_left_joystick(env.console, 128, 255, 16, 0);
        pbf_move_left_joystick(env.console, 0, 255, 16, 0);
    }
}
void ShinyHuntAutonomousOverworld::move_in_circle_down(
    SingleSwitchProgramEnvironment& env,
    bool counter_clockwise
) const{
    if (counter_clockwise){
        pbf_move_left_joystick(env.console, 0, 128, 16, 0);
        pbf_move_left_joystick(env.console, 0, 255, 16, 0);
        pbf_move_left_joystick(env.console, 128, 255, 16, 0);
        pbf_move_left_joystick(env.console, 255, 255, 16, 0);
        pbf_move_left_joystick(env.console, 255, 128, 16, 0);
        pbf_move_left_joystick(env.console, 255, 0, 32, 0);
        pbf_move_left_joystick(env.console, 128, 0, 32, 0);
        pbf_move_left_joystick(env.console, 0, 0, 32, 0);
    }else{
        pbf_move_left_joystick(env.console, 255, 128, 16, 0);
        pbf_move_left_joystick(env.console, 255, 255, 16, 0);
        pbf_move_left_joystick(env.console, 128, 255, 16, 0);
        pbf_move_left_joystick(env.console, 0, 255, 16, 0);
        pbf_move_left_joystick(env.console, 0, 128, 16, 0);
        pbf_move_left_joystick(env.console, 0, 0, 32, 0);
        pbf_move_left_joystick(env.console, 128, 0, 32, 0);
        pbf_move_left_joystick(env.console, 255, 0, 32, 0);
    }
}
void ShinyHuntAutonomousOverworld::circle_in_place(
    SingleSwitchProgramEnvironment& env,
    bool counter_clockwise
) const{
    if (counter_clockwise){
        pbf_move_left_joystick(env.console, 0, 128, 64, 0);     //  Correct for bias.
        pbf_move_left_joystick(env.console, 128, 255, 32, 0);
        pbf_move_left_joystick(env.console, 255, 255, 32, 0);
        pbf_move_left_joystick(env.console, 255, 128, 32, 0);
        pbf_move_left_joystick(env.console, 255, 0, 32, 0);
        pbf_move_left_joystick(env.console, 128, 0, 32, 0);
        pbf_move_left_joystick(env.console, 0, 0, 32, 0);
        pbf_move_left_joystick(env.console, 0, 128, 32, 0);
        pbf_move_left_joystick(env.console, 0, 255, 32, 0);
        pbf_move_left_joystick(env.console, 255, 128, 16, 0);   //  Correct for bias.
    }else{
        pbf_move_left_joystick(env.console, 255, 128, 64, 0);   //  Correct for bias.
        pbf_move_left_joystick(env.console, 128, 255, 32, 0);
        pbf_move_left_joystick(env.console, 0, 255, 32, 0);
        pbf_move_left_joystick(env.console, 0, 128, 32, 0);
        pbf_move_left_joystick(env.console, 0, 0, 32, 0);
        pbf_move_left_joystick(env.console, 128, 0, 32, 0);
        pbf_move_left_joystick(env.console, 255, 0, 32, 0);
        pbf_move_left_joystick(env.console, 255, 128, 32, 0);
        pbf_move_left_joystick(env.console, 255, 255, 32, 0);
        pbf_move_left_joystick(env.console, 0, 128, 16, 0);     //  Correct for bias.
    }
}


MarkWatchResult ShinyHuntAutonomousOverworld::whistle_and_watch(
    SingleSwitchProgramEnvironment& env,
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions
) const{
    MarkTracker mark_tracker(env, env.console);

    const double center_x = 0.5;
    const double center_y = 0.70;
    InferenceBoxScope self(env.console, Qt::cyan, center_x - 0.02, center_y - 0.05, 0.04, 0.1);

    //  Whistle
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 0);

    return mark_tracker.watch_for(
        exclamations, questions,
        std::chrono::milliseconds(1000)
    );
}


enum class OverworldMark{
    EXCLAMATION_MARK,
    QUESTION_MARK,
};

struct OverworldTarget{
    OverworldMark mark;
    InferenceBox box;
    Trajectory trajectory;
    double delta_x;
    double delta_y;
};


void populate_targets(
    std::multimap<double, OverworldTarget>& scored_targets,
    const std::vector<OverworldTarget>& targets
){
#if 0
    cout << "Targets:" << endl;
    for (const auto& item : targets){
        cout << "    " << item.box.x << " - " << item.box.x + item.box.width
             << " x " << item.box.y << " - " << item.box.y + item.box.height << endl;
    }
#endif

//    cout << "Candidates:" << endl;
    for (size_t c = 0; c < targets.size(); c++){
        double overlap = 0;
        const InferenceBox& box0 = targets[c].box;
        for (size_t i = 0; i < targets.size(); i++){
            const InferenceBox& box1 = targets[i].box;
            double min_x = std::max(box0.x, box1.x);
            double max_x = std::min(box0.x + box0.width, box1.x + box1.width);
            if (min_x >= max_x){
                continue;
            }
            double min_y = std::max(box0.y, box1.y);
            double max_y = std::min(box0.y + box0.height, box1.y + box1.height);
            if (min_y >= max_y){
                continue;
            }
            overlap += (max_x - min_x) * (max_y - min_y);
        }
        double score = targets[c].trajectory.distance_in_ticks / overlap;
        scored_targets.emplace(score, targets[c]);
//        cout << "    " << score << " = "
//             << (int)targets[c].trajectory.joystick_x << ", "
//             << (int)targets[c].trajectory.joystick_y << endl;
    }
}


const double OVERWORLD_CENTER_X = 0.50;
const double OVERWORLD_CENTER_Y = 0.70;

void populate_targets(
    std::multimap<double, OverworldTarget>& scored_targets,
    ProgramEnvironment& env,
    const std::vector<InferenceBox>& marks,
    double offset,
    OverworldMark mark
){
    std::vector<OverworldTarget> targets;
    for (const InferenceBox& box : marks){
        double delta_x = box.x + box.width / 2 - OVERWORLD_CENTER_X;
        double delta_y = box.y + box.height * (1.0 + offset) - OVERWORLD_CENTER_Y;
        Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
        targets.emplace_back(OverworldTarget{mark, box, trajectory, delta_x, delta_y});
    }
    populate_targets(scored_targets, targets);
}



bool ShinyHuntAutonomousOverworld::find_encounter(
    SingleSwitchProgramEnvironment& env,
    Stats& stats,
    StandardEncounterTracker& tracker
) const{
    InferenceBoxScope self(
        env.console, Qt::cyan,
        OVERWORLD_CENTER_X - 0.02,
        OVERWORLD_CENTER_Y - 0.05,
        0.04, 0.1
    );

    const std::chrono::milliseconds TIMEOUT((uint64_t)WATCHDOG_TIMER * 1000 / TICKS_PER_SECOND);

    size_t nothing_found_counter = 0;
    std::vector<InferenceBox> exclamation_marks;
    std::vector<InferenceBox> question_marks;

    auto last = std::chrono::system_clock::now();
    while (true){
        //  No battle for a long time. Reset the game.
        auto now = std::chrono::system_clock::now();
        if (now - last > TIMEOUT){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
            stats.m_resets++;
            return false;
        }

        //  If there's nothing, whistle and watch.
        if (exclamation_marks.empty() && question_marks.empty()){
            MarkWatchResult result = whistle_and_watch(
                env,
                exclamation_marks,
                question_marks
            );
            switch (result){
            case MarkWatchResult::BATTLE_MENU:
                stats.m_errors++;
                tracker.run_away();
                return false;
            case MarkWatchResult::BATTLE_START:
                env.log("Battle started! (whistle)");
                return true;
            default:;
            }
        }

//        cout << "exclamation_marks = " << exclamation_marks.size() << endl;
//        cout << "question_marks    = " << question_marks.size() << endl;

        AsyncStartBattleDetector start_battle(env, env.console);

        //  Build targets.
        std::multimap<double, OverworldTarget> targets;
        {
            populate_targets(targets, env, exclamation_marks, MARK_OFFSET, OverworldMark::EXCLAMATION_MARK);
            if (targets.empty() || !PRIORITIZE_EXCLAMATION_POINTS){
                populate_targets(targets, env, question_marks, MARK_OFFSET, OverworldMark::QUESTION_MARK);
            }
            exclamation_marks.clear();
            question_marks.clear();
        }

//        std::unique_ptr<AsyncMarkTracker> tracker(new AsyncMarkTracker(
//            env, env.console, exclamation_marks, question_marks
//        ));

        //  Pick a target.
        auto target = targets.end();
        if (!targets.empty()){
            target = targets.begin();
            env.log(
                QString("Best Target: ") +
                (target->second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
                " at [" +
                QString::number(target->second.delta_x) + " , " +
                QString::number(-target->second.delta_y) + "], alpha = " +
                QString::number(target->first),
                "purple"
            );
        }

        //  No good target found. Rotate the view and try again.
        if (targets.empty() || target->first > MAX_TARGET_ALPHA){
            if (targets.empty()){
                env.log("No targets found.", "orange");
            }else{
                env.log("Target is too weak.", "orange");
            }

            env.log("Nothing found. Rotating view.");
            nothing_found_counter++;
            if (LOCAL_CIRCLING != 0 && nothing_found_counter >= LOCAL_CIRCLING){
                circle_in_place(env, rand() % 2);
                nothing_found_counter = 0;
            }else{
                pbf_move_right_joystick(env.console, 192, 255, 50, 70);
            }
            env.console.botbase().wait_for_all_requests();
//            if (tracker->result() == MarkWatchResult::BATTLE_START){
            if (start_battle.detected()){
                env.log("Battle started! (rotate)");
                return true;
            }
            continue;
        }

        nothing_found_counter = 0;


        //  Pick a target.
        target->second.box.color = Qt::yellow;
        InferenceBoxScope target_box(env.console, target->second.box);
        const Trajectory& trajectory = target->second.trajectory;

        double angle = std::atan2(
            (double)trajectory.joystick_y - 128,
            (double)trajectory.joystick_x - 128
        ) * 57.295779513082320877;
        env.log(
            "Trajectory: Distance = " + QString::number(trajectory.distance_in_ticks) +
            ", Direction = " + QString::number(-angle) + " degrees"
        );



        //  Move towards target.

        int duration = trajectory.distance_in_ticks + 16;
        if (duration > (int)MAX_MOVE_DURATION){
            duration = MAX_MOVE_DURATION;
        }
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
                move_in_circle_up(
                    env,
                    trajectory.joystick_x > 128
                );
            }else{
                move_in_circle_down(
                    env,
                    trajectory.joystick_x <= 128
                );
            }
        }

        env.console.botbase().wait_for_all_requests();
//        if (tracker->result() == MarkWatchResult::BATTLE_START){
        if (start_battle.detected()){
            env.log("Battle started! (move)");
            exclamation_marks.clear();
            question_marks.clear();
            return true;
        }
    }
}

void ShinyHuntAutonomousOverworld::program(SingleSwitchProgramEnvironment& env) const{
    srand(time(nullptr));

    grip_menu_connect_go_home(env.console);
    resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    pbf_move_right_joystick(env.console, 128, 255, TICKS_PER_SECOND, 0);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    StandardEncounterTracker tracker(
        stats, env.console,
        false,
        EXIT_BATTLE_MASH_TIME,
        VIDEO_ON_SHINY,
        RUN_FROM_EVERYTHING
    );

    //  Encounter Loop
    while (true){
        env.update_stats();

        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }
        env.console.botbase().wait_for_all_requests();

        bool battle = find_encounter(env, stats, tracker);
        if (!battle){
            continue;
        }

        //  Detect shiny.
        ShinyDetection detection = detect_shiny_battle(
            env, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyDetection::NO_BATTLE_MENU){
            stats.m_errors++;
            pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

