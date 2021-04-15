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
        true
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
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld.",
        "6 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
{
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");
    m_options.emplace_back(&PRIORITIZE_EXCLAMATION_POINTS, "PRIORITIZE_EXCLAMATION_POINTS");
    m_options.emplace_back(&TARGET_CIRCLING, "ENABLE_CIRCLING");
    m_options.emplace_back(&LOCAL_CIRCLING, "LOCAL_CIRCLING");
    m_options.emplace_back(&MAX_MOVE_DURATION, "MAX_MOVE_DURATION");
    m_options.emplace_back(&WATCHDOG_TIMER, "WATCHDOG_TIMER");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
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




void ShinyHuntAutonomousOverworld::move_in_circle(
    SingleSwitchProgramEnvironment& env,
    uint8_t size_ticks,
    uint8_t current_direction_x,
    uint8_t current_direction_y
) const{
//    cout << "size_ticks = " << (int)size_ticks << endl;

    if (current_direction_x <= 128){
        pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0);     //  Correct for bias.
        pbf_move_left_joystick(env.console, 128, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 128, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 128, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 128, size_ticks, 0);   //  Correct for bias.
    }else{
        pbf_move_left_joystick(env.console, 255, 128, size_ticks, 0);   //  Correct for bias.
        pbf_move_left_joystick(env.console, 128, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 128, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 255, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 128, size_ticks, 0);
        pbf_move_left_joystick(env.console, 255, 0, size_ticks, 0);
        pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0);     //  Correct for bias.
    }
}


ShinyHuntAutonomousOverworld::WatchResult ShinyHuntAutonomousOverworld::whistle_and_watch(
    SingleSwitchProgramEnvironment& env,
    std::vector<InferenceBox>& exclamations,
    std::vector<InferenceBox>& questions
) const{
    StandardBattleMenuDetector battle_menu(env.console);
    StartBattleDetector start_battle(env.console, std::chrono::milliseconds(0));
    InferenceBoxScope search_area(env.console, 0.0, 0.2, 1.0, 0.8);

    const double center_x = 0.5;
    const double center_y = 0.70;
    InferenceBoxScope self(env.console, Qt::cyan, center_x - 0.02, center_y - 0.05, 0.04, 0.1);


    //  Whistle
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 0);

    std::deque<InferenceBoxScope> detection_boxes;

    size_t count = 0;

    InferenceThrottler throttler(std::chrono::milliseconds(1000), std::chrono::milliseconds(50));
    while (true){
        env.check_stopping();

        QImage screen = env.console.video().snapshot();

        //  Check if a battle has started.
        if (battle_menu.detect(screen)){
            return WatchResult::BATTLE_MENU;
        }
        if (start_battle.detect(screen)){
            return WatchResult::BATTLE_START;
        }

        //  Look for exclamation points and question marks.
        QImage search_image = extract_box(screen, search_area);

        std::vector<PixelBox> exclamation_marks;
        std::vector<PixelBox> question_marks;
        count += find_marks(
            search_image,
            &exclamation_marks,
            &question_marks
        );

        detection_boxes.clear();
        for (const PixelBox& mark : exclamation_marks){
            InferenceBox box = translate_to_parent(screen, search_area, mark);
            box.color = Qt::magenta;
            box.x -= box.width;
            box.width *= 3;
            box.height *= 2;
            detection_boxes.emplace_back(env.console, box);
            exclamations.emplace_back(box);
        }
        for (const PixelBox& mark : question_marks){
            InferenceBox box = translate_to_parent(screen, search_area, mark);
            box.color = Qt::magenta;
            box.x -= box.width / 2;
            box.width *= 2;
            box.height *= 2;
            detection_boxes.emplace_back(env.console, box);
            questions.emplace_back(box);
        }

        if (throttler.end_iteration(env)){
            return WatchResult::TIMEOUT;
        }
    }
}


bool ShinyHuntAutonomousOverworld::find_encounter(
    SingleSwitchProgramEnvironment& env,
    Stats& stats,
    StandardEncounterTracker& tracker
) const{
    const double center_x = 0.5;
    const double center_y = 0.70;
    InferenceBoxScope self(env.console, Qt::cyan, center_x - 0.02, center_y - 0.05, 0.04, 0.1);

    const std::chrono::milliseconds TIMEOUT((uint64_t)WATCHDOG_TIMER * 1000 / TICKS_PER_SECOND);

    size_t nothing_found_counter = 0;

    auto last = std::chrono::system_clock::now();
    while (true){
        //  No battle for a long time. Reset the game.
        auto now = std::chrono::system_clock::now();
        if (now - last > TIMEOUT){
            pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            reset_game_from_home_with_inference(
                env, env.console,
                TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
            stats.m_resets++;
            return false;
        }

        std::vector<InferenceBox> exclamation_marks;
        std::vector<InferenceBox> question_marks;
        WatchResult result = whistle_and_watch(
            env,
            exclamation_marks,
            question_marks
        );
        switch (result){
        case WatchResult::BATTLE_MENU:
            stats.m_errors++;
            tracker.run_away();
            return false;
        case WatchResult::BATTLE_START:
            env.log("Battle started!");
            return true;
        default:;
        }

        //  Nothing was found. Rotate the view and try again.
        if (exclamation_marks.empty() && question_marks.empty()){
            env.log("Nothing found. Rotating view.");
            nothing_found_counter++;
            if (LOCAL_CIRCLING != 0 && nothing_found_counter >= LOCAL_CIRCLING){
                move_in_circle(env, 32, 0, 0);
                nothing_found_counter = 0;
            }else{
                pbf_move_right_joystick(192, 255, 50, 70);
            }
            env.console.botbase().wait_for_all_requests();
            continue;
        }

        nothing_found_counter = 0;

        std::multimap<uint16_t, std::pair<Trajectory, InferenceBox>> exclamations;
        for (const InferenceBox& box : exclamation_marks){
            double delta_x = box.x + box.width / 2 - center_x;
            double delta_y = box.y + box.height * 1.5 - center_y;
            env.log(
                "Exclamation at: [" + QString::number(delta_x) + " , " + QString::number(-delta_y) + "]",
                "purple"
            );
            Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
            exclamations.emplace(trajectory.distance_in_ticks, std::pair(trajectory, box));
            break;
        }

        std::multimap<uint16_t, std::pair<Trajectory, InferenceBox>> questions;
        for (const InferenceBox& box : question_marks){
            double delta_x = box.x + box.width / 2 - center_x;
            double delta_y = box.y + box.height * 1.5 - center_y;
            env.log(
                "Question at: [" + QString::number(delta_x) + " , " + QString::number(-delta_y) + "]",
                "purple"
            );
            Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
            questions.emplace(trajectory.distance_in_ticks, std::pair(trajectory, box));
            break;
        }


        //  Pick a target.
        std::pair<Trajectory, InferenceBox> target;
        target.first.distance_in_ticks = (uint16_t)0 - 1;

        if (PRIORITIZE_EXCLAMATION_POINTS && !exclamations.empty()){
            questions.clear();
        }
        if (!exclamations.empty() && target.first.distance_in_ticks > exclamations.begin()->first){
            target = exclamations.begin()->second;
        }
        if (!questions.empty() && target.first.distance_in_ticks > questions.begin()->first){
            target = questions.begin()->second;
        }

        target.second.color = Qt::yellow;
        InferenceBoxScope target_box(env.console, target.second);

        double angle = std::atan2(
            (double)target.first.joystick_y - 128,
            (double)target.first.joystick_x - 128
        ) * 57.295779513082320877;
        env.log(
            "Found something. Distance: " + QString::number(target.first.distance_in_ticks) +
            ", Direction: " + QString::number(-angle) + " degrees"
        );



        //  Move towards target.

        int duration = target.first.distance_in_ticks + 30;
        if (duration > (int)MAX_MOVE_DURATION){
            duration = MAX_MOVE_DURATION;
        }
        pbf_move_left_joystick(
            target.first.joystick_x,
            target.first.joystick_y,
            (uint16_t)duration, 0
        );

        //  Circle Maneuver
        if (TARGET_CIRCLING){
            move_in_circle(
                env, 16,
                target.first.joystick_x,
                target.first.joystick_y
            );
        }

        env.console.botbase().wait_for_all_requests();
    }
}

void ShinyHuntAutonomousOverworld::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    pbf_move_right_joystick(128, 255, TICKS_PER_SECOND, 0);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock();

    Stats& stats = env.stats<Stats>();
    StandardEncounterTracker tracker(stats, env.console, false, EXIT_BATTLE_MASH_TIME);

    //  Encounter Loop
    while (true){
        env.update_stats();

        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock() - last_touch >= PERIOD){
            pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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
            pbf_mash_button(BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback();
    end_program_loop();
}



}
}
}

