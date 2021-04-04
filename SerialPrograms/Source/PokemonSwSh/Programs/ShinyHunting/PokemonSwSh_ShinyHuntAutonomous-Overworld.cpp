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
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
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
    , ENABLE_CIRCLING(
        "<b>Enable Circling:</b><br>After moving towards a " + STRING_POKEMON + ", make a circle."
        " This increases the chance of encountering the " + STRING_POKEMON + " if it has moved or if the trajectory missed.",
        true
    )
    , MAX_MOVE_DURATION(
        "<b>Maximum Move Duration:</b><br>Do not move in the same direction for more than this long."
        " If you set this too high, you may wander too far from the grassy area.",
        "150"
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
    m_options.emplace_back(&ENABLE_CIRCLING, "ENABLE_CIRCLING");
    m_options.emplace_back(&MAX_MOVE_DURATION, "MAX_MOVE_DURATION");
    m_options.emplace_back(&WATCHDOG_TIMER, "WATCHDOG_TIMER");
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
}


std::string ShinyHuntAutonomousOverworld::Stats::stats() const{
    std::string str;
    str += str_encounters();
    str += " - Timeouts: " + tostr_u_commas(m_timeouts);
    str += " - Unexpected Battles: " + tostr_u_commas(m_unexpected_battles);
    str += " - Resets: " + tostr_u_commas(m_resets);
    str += str_shinies();
    return str;
}




struct Trajectory{
    uint16_t distance_in_ticks;
    uint8_t joystick_x;
    uint8_t joystick_y;
};

const Trajectory TRAJECTORY_TABLE[8][6] = {
    {{400,128,  0},{420,160,  0},{420,190,  0},{480,208,  0},{480,208,  0},{480,208,  0},},
    {{210,128,  0},{220,164,  0},{250,200,  0},{280,208,  0},{300,209,  0},{320,209,  0},},
    {{120,128,  0},{120,192,  0},{140,209,  0},{180,224,  0},{220,224,  0},{260,232,  0},},
    {{ 60,128,  0},{ 65,224,  0},{ 95,255, 32},{125,255, 48},{160,255, 48},{200,255, 48},},
    {{  0,128,128},{ 40,255,110},{ 70,255,105},{100,255,100},{130,255, 95},{160,255, 90},},
    {{ 35,128,255},{ 45,255,255},{ 70,255,170},{ 90,255,160},{115,255,140},{140,255,128},},
    {{ 55,128,255},{ 60,192,255},{ 75,255,255},{ 90,255,192},{110,255,176},{135,255,144},},
    {{ 75,128,255},{ 80,160,255},{ 85,216,255},{ 95,255,224},{115,255,192},{130,255,176},},
};
Trajectory get_trajectory_int(int delta_x, int delta_y){
    delta_x = std::max(delta_x, -5);
    delta_x = std::min(delta_x, +5);
    delta_y = std::max(delta_y, -4);
    delta_y = std::min(delta_y, +3);

    if (delta_x < 0){
        Trajectory entry = get_trajectory_int(-delta_x, delta_y);
        entry.joystick_x = 256 - entry.joystick_x;
        return entry;
    }

    return TRAJECTORY_TABLE[delta_y + 4][delta_x];
}
Trajectory get_trajectory_float(double delta_x, double delta_y){
    delta_x *= 10;
    delta_y *= 10;

    int int_x = std::floor(delta_x);
    int int_y = std::floor(delta_y);

    double frac_x = delta_x - int_x;
    double frac_y = delta_y - int_y;

    Trajectory corner11 = get_trajectory_int(int_x, int_y);
    Trajectory corner01 = get_trajectory_int(int_x + 1, int_y);
    Trajectory corner10 = get_trajectory_int(int_x, int_y + 1);
    Trajectory corner00 = get_trajectory_int(int_x + 1, int_y + 1);

    double top_x = corner11.joystick_x * (1.0 - frac_x) + corner01.joystick_x * frac_x;
    double bot_x = corner10.joystick_x * (1.0 - frac_x) + corner00.joystick_x * frac_x;
    double x = top_x * (1.0 - frac_y) + bot_x * frac_y;
//    cout << "top_x = " << top_x << endl;
//    cout << "bot_x = " << bot_x << endl;
//    cout << "x = " << x << endl;

    double top_y = corner11.joystick_y * (1.0 - frac_y) + corner10.joystick_y * frac_y;
    double bot_y = corner01.joystick_y * (1.0 - frac_y) + corner00.joystick_y * frac_y;
    double y = top_y * (1.0 - frac_x) + bot_y * frac_x;

    double top_d = corner11.distance_in_ticks * (1.0 - frac_x) + corner01.distance_in_ticks * frac_x;
    double bot_d = corner10.distance_in_ticks * (1.0 - frac_x) + corner00.distance_in_ticks * frac_x;
    double d = top_d * (1.0 - frac_y) + bot_d * frac_y;

    x -= 128;
    y -= 128;
    double scale = std::max(std::abs(x), std::abs(y));
    if (scale == 0){
        x = 128;
        y = 128;
    }else{
        scale = 128 / scale;
        x *= scale;
        y *= scale;
    }
    x += 128;
    y += 128;

    d = std::max(d, 0.0);
    d = std::min(d, 65535.);
    x = std::max(x, 0.0);
    y = std::max(y, 0.0);
    x = std::min(x, 255.);
    y = std::min(y, 255.);

    return Trajectory{(uint16_t)d, (uint8_t)x, (uint8_t)y};
}


void ShinyHuntAutonomousOverworld::move_in_circle(SingleSwitchProgramEnvironment& env, uint8_t size_ticks) const{
//    cout << "size_ticks = " << (int)size_ticks << endl;

    pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0); //  Correct for bias.

    size_ticks *= 2;
    pbf_move_left_joystick(env.console, 128, 0, size_ticks, 0);
    pbf_move_left_joystick(env.console, 255, 0, size_ticks, 0);
    pbf_move_left_joystick(env.console, 255, 128, size_ticks, 0);
    pbf_move_left_joystick(env.console, 255, 255, size_ticks, 0);
    pbf_move_left_joystick(env.console, 128, 255, size_ticks, 0);
    pbf_move_left_joystick(env.console, 0, 255, size_ticks, 0);
    pbf_move_left_joystick(env.console, 0, 128, size_ticks, 0);
    pbf_move_left_joystick(env.console, 0, 0, size_ticks, 0);
}
bool ShinyHuntAutonomousOverworld::find_encounter(
    SingleSwitchProgramEnvironment& env,
    Stats& stats,
    StandardEncounterTracker& tracker
) const{
    InferenceBoxScope search_area(env.console, 0.0, 0.2, 1.0, 0.8);
    StandardBattleMenuDetector battle_menu(env.console);
    StartBattleDetector start_battle(env.console, std::chrono::milliseconds(0));

    const double center_x = 0.5;
    const double center_y = 0.70;
    InferenceBoxScope self(env.console, Qt::cyan, center_x - 0.02, center_y - 0.05, 0.04, 0.1);

    const std::chrono::milliseconds TIMEOUT((uint64_t)WATCHDOG_TIMER * 1000 / TICKS_PER_SECOND);

    auto last = std::chrono::system_clock::now();
//    size_t c = 0;
    while (true){
        std::deque<InferenceBoxScope> boxes;
        InferenceThrottler throttler(std::chrono::seconds(1));

        std::multimap<uint16_t, Trajectory> detections;
        do{
            env.check_stopping();

            //  No battle for a long time. Reset the game.
            auto now = std::chrono::system_clock::now();
            if (now - last > TIMEOUT){
                pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
                reset_game_from_home_with_inference(
                    env, env.logger,
                    env.console,
                    TOLERATE_SYSTEM_UPDATE_MENU_FAST
                );
                stats.m_resets++;
                return false;
            }

            //  Grab screenshot.
            QImage screen = env.console.video().snapshot();
//            screen.save("test-" + QString::number(c++) + ".png");

            //  Check if a battle has started.
            if (battle_menu.detect(screen)){
                stats.m_unexpected_battles++;
                tracker.run_away();
                return false;
            }
            if (start_battle.detect(screen)){
                env.logger.log("Battle started!");
                return true;
            }

            //  Look for exclamation points and question marks.
            QImage search_image = extract_box(screen, search_area);

            std::vector<PixelBox> exclamation_marks;
            std::vector<PixelBox> question_marks;

            size_t count = find_marks(
                search_image,
                &exclamation_marks,
                &question_marks
            );
            if (count == 0){
                pbf_press_button(BUTTON_LCLICK, 5, 0);
//                move_in_circle(env, 5);
                env.console.botbase().wait_for_all_requests();
                boxes.clear();
                continue;
            }

            boxes.clear();
            for (const PixelBox& mark : exclamation_marks){
                InferenceBox box = translate_to_parent(screen, search_area, mark);
                box.color = Qt::magenta;
                box.x -= box.width;
                box.width *= 3;
                box.height *= 2;
                boxes.emplace_back(env.console, box);

                double delta_x = box.x + box.width / 2 - center_x;
                double delta_y = box.y + box.height * 1.5 - center_y;
//                double distance = delta_x*delta_x + delta_y*delta_y;
                env.logger.log(
                    "Exclamation at: [" + QString::number(delta_x) + " , " + QString::number(-delta_y) + "]",
                    "purple"
                );
    //            cout << std::sqrt(distance) << endl;
                Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
                detections.emplace(trajectory.distance_in_ticks, trajectory);
                break;
            }
            for (const PixelBox& mark : question_marks){
                InferenceBox box = translate_to_parent(screen, search_area, mark);
                box.color = Qt::magenta;
                box.x -= box.width / 2;
                box.width *= 2;
                box.height *= 2;
                boxes.emplace_back(env.console, box);

                double delta_x = box.x + box.width / 2 - center_x;
                double delta_y = box.y + box.height * 1.5 - center_y;
//                double distance = delta_x*delta_x + delta_y*delta_y;
                env.logger.log(
                    "Question at: [" + QString::number(delta_x) + " , " + QString::number(-delta_y) + "]",
                    "purple"
                );
    //            cout << std::sqrt(distance) << endl;
                if (!PRIORITIZE_EXCLAMATION_POINTS || exclamation_marks.empty()){
                    Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
                    detections.emplace(trajectory.distance_in_ticks, trajectory);
                }
                break;
            }
            if (!detections.empty()){
                break;
            }

        }while (!throttler.end_iteration(env));

        //  Nothing was found. Rotate the view and try again.
        if (detections.empty()){
            env.logger.log("Nothing found. Rotating view.");
            pbf_move_right_joystick(192, 255, 50, 0);
//            move_in_circle(env, 5);
            env.console.botbase().wait_for_all_requests();
            continue;
        }

        auto target = detections.begin();
        const Trajectory& trajectory = target->second;

        double angle = std::atan2((double)trajectory.joystick_y - 128, (double)trajectory.joystick_x - 128) * 57.295779513082320877;
        env.logger.log("Found something. Distance: " + QString::number(trajectory.distance_in_ticks) + ", Direction: " + QString::number(-angle) + " degrees");

        int duration = trajectory.distance_in_ticks + 30;
        if (duration > (int)MAX_MOVE_DURATION){
            duration = MAX_MOVE_DURATION;
        }
        pbf_wait(50);
        pbf_move_left_joystick(
            trajectory.joystick_x,
            trajectory.joystick_y,
            (uint16_t)duration, 0
        );

        //  Circle Maneuver
        if (ENABLE_CIRCLING){
            move_in_circle(env, 8);
        }

        env.console.botbase().wait_for_all_requests();
//        boxes.clear();
    }
}

void ShinyHuntAutonomousOverworld::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    pbf_move_right_joystick(128, 255, TICKS_PER_SECOND, 0);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock();

    Stats stats;
    StandardEncounterTracker tracker(stats, env.console, false, EXIT_BATTLE_MASH_TIME);

    //  Encounter Loop
    while (true){
        stats.log_stats(env, env.logger);

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
            env, env.console, env.logger,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyDetection::NO_BATTLE_MENU){
            stats.m_timeouts++;
            pbf_mash_button(BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }
    }

    stats.log_stats(env, env.logger);

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback();
    end_program_loop();
}



}
}
}

