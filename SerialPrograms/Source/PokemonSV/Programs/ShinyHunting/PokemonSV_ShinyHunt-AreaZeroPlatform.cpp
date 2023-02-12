/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
//#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
//#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/PokemonSV_Settings.h"
//#include "PokemonSV/Inference/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
//#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_ShinyHunt-AreaZeroPlatform.h"

#include <iostream>
using std::cout;
using std::endl;

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
struct ShinyHuntAreaZeroPlatform_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_kills(m_stats["Kills"])
        , m_encounters(m_stats["Encounters"])
        , m_sandwiches(m_stats["Sandwiches"])
        , m_platform_resets(m_stats["Platform Resets"])
        , m_game_resets(m_stats["Game Resets"])
        , m_errors(m_stats["Errors"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Kills");
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Sandwiches", true);
        m_display_order.emplace_back("Platform Resets", true);
        m_display_order.emplace_back("Game Resets", true);
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& m_kills;
    std::atomic<uint64_t>& m_encounters;
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_platform_resets;
    std::atomic<uint64_t>& m_game_resets;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntAreaZeroPlatform_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




ShinyHuntAreaZeroPlatform::ShinyHuntAreaZeroPlatform()
    : MODE(
        "<b>Mode:</b><br>"
        "If starting on the platform, you should stand near the center of the platform facing any direction.<br>"
        "If starting in the Zero Gate, you should be just inside the building as if you just entered.<br>"
        "If making a sandwich, you should be at the Zero Gate fly spot as if you just flew there.",
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
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        LockWhileRunning::UNLOCKED,
        true
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_SHINY(
        "Shiny Encounter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(MODE);
        PA_ADD_OPTION(PATH0);
    }
    PA_ADD_OPTION(VIDEO_ON_SHINY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(PLATFORM_RESET);
        PA_ADD_OPTION(NAVIGATE_TO_PLATFORM);
    }
    PA_ADD_OPTION(NOTIFICATIONS);
}





bool ShinyHuntAreaZeroPlatform::clear_in_front(
    BotBaseContext& context,
    bool throw_ball_if_bubble,
    std::function<void(BotBaseContext& context)>&& command
){
//    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
    ConsoleHandle& console = m_env->console;

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

    WallClock last_kill = m_kill_watcher->last_kill();
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
        if (last_kill == m_kill_watcher->last_kill()){
//            cout << "no kill" << endl;
            break;
        }
//        cout << "found kill" << endl;
        last_kill = m_kill_watcher->last_kill();
    }
    console.log("Nothing left to clear...");
    return m_kill_watcher->last_kill() != WallClock::min();
}

void ShinyHuntAreaZeroPlatform::run_path0(BotBaseContext& context){
    ConsoleHandle& console = m_env->console;

    //  Go back to the wall.
    console.log("Go back to wall...");
    clear_in_front(context, false, [&](BotBaseContext& context){
        find_and_center_on_sky(*m_env, console, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 176, 255, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    clear_in_front(context, false, [&](BotBaseContext& context){
        //  Move to wall.
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 0);

        //  Turn around.
        console.log("Turning towards sky...");
        pbf_move_left_joystick(context, 128, 255, 30, 95);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    //  Move forward and kill everything in your path.
    console.log("Moving towards sky and killing everything...");
    uint16_t duration = 325;
    clear_in_front(context, true, [&](BotBaseContext& context){
        find_and_center_on_sky(*m_env, console, context);
        pbf_move_right_joystick(context, 128, 255, 70, 0);

        uint8_t x = 128;
        switch (m_iterations % 4){
        case 0:
            x = 96;
            duration = 250;
            break;
        case 1:
            x = 112;
            break;
        case 2:
            x = 128;
            break;
        case 3:
            x = 112;
            break;
        }

        ssf_press_button(context, BUTTON_L, 0, 20);
        pbf_move_left_joystick(context, x, 0, duration, 0);
    });
    clear_in_front(context, true, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
    });
}
void ShinyHuntAreaZeroPlatform::run_path1(BotBaseContext& context){
    ConsoleHandle& console = m_env->console;

    //  Go back to the wall.
    console.log("Go back to wall...");
    pbf_press_button(context, BUTTON_L, 20, 105);
    clear_in_front(context, true, [&](BotBaseContext& context){
        find_and_center_on_sky(*m_env, console, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 192, 255, 60, 0);
    });

    //  Clear path to the wall.
    console.log("Clear path to the wall...");
    pbf_press_button(context, BUTTON_L, 20, 50);
    clear_in_front(context, false, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 128, 0, 5 * TICKS_PER_SECOND, 0);

        //  Turn right.
        pbf_move_left_joystick(context, 255, 128, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    //  Clear the wall.
    console.log("Clear the wall...");
    uint16_t duration = 325;
    clear_in_front(context, true, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 255, 128, 125, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
        context.wait_for_all_requests();

        find_and_center_on_sky(*m_env, console, context);
        pbf_move_left_joystick(context, 128, 0, 50, 0);
        pbf_press_button(context, BUTTON_L, 20, 30);

        //  Move forward.

        uint8_t x = 128;
        switch (m_iterations % 4){
        case 0:
            x = 96;
            duration = 250;
            break;
        case 1:
            x = 112;
            break;
        case 2:
            x = 128;
            break;
        case 3:
            x = 112;
            break;
        }

        pbf_move_left_joystick(context, x, 0, duration, 0);
    });

    console.log("Run backwards and wait...");
    clear_in_front(context, true, [&](BotBaseContext& context){
//        pbf_move_left_joystick(context, 64, 0, 125, 0);
//        pbf_press_button(context, BUTTON_L, 20, 105);
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
//        pbf_controller_state(context, 0, DPAD_NONE, 255, 255, 120, 128, 3 * TICKS_PER_SECOND);
    });
}



void direction_to_stick(
    uint8_t& joystick_x, uint8_t& joystick_y,
    double direction_x, double direction_y
){
//    cout << "direction = " << direction_x << ", " << direction_y << endl;

    double scale = std::max(std::abs(direction_x), std::abs(direction_y));
    direction_x = 128 / scale * direction_x + 128;
    direction_y = 128 / scale * direction_y + 128;

//    cout << "joystick = " << direction_x << ", " << direction_y << endl;

    direction_x = std::min(direction_x, 255.);
    direction_x = std::max(direction_x, 0.);
    direction_y = std::min(direction_y, 255.);
    direction_y = std::max(direction_y, 0.);

    joystick_x = (double)direction_x;
    joystick_y = (double)direction_y;
}
void choose_path(
    Logger& logger,
    uint8_t& x, uint8_t& y, uint16_t& duration,
    double platform_x, double platform_y
){
    double diff_x = platform_x - 0.65;
    double diff_y = platform_y - 0.70;

    logger.log("Move Direction: x = " + tostr_default(diff_x) + ", y = " + tostr_default(diff_y), COLOR_BLUE);

    direction_to_stick(x, y, diff_x, diff_y);
    duration = (uint16_t)std::min<double>(std::sqrt(diff_x*diff_x + diff_y*diff_y) * 125 * 12, 400);
}
void turn_angle(BotBaseContext& context, double angle_radians){
    uint8_t turn_x, turn_y;
    direction_to_stick(turn_x, turn_y, -std::sin(angle_radians), std::cos(angle_radians));
    pbf_move_left_joystick(context, turn_x, turn_y, 40, 20);
    pbf_mash_button(context, BUTTON_L, 60);
}

void ShinyHuntAreaZeroPlatform::run_path2(BotBaseContext& context){
    ConsoleHandle& console = m_env->console;

    console.log("Look forward and fire...");
    pbf_mash_button(context, BUTTON_L, 60);

    double platform_x, platform_y;
    uint16_t duration;
    uint8_t move_x, move_y;
    clear_in_front(context, true, [&](BotBaseContext& context){

        console.log("Find the sky, turn around and fire.");
        pbf_move_right_joystick(context, 128, 0, 60, 0);
        find_and_center_on_sky(*m_env, console, context);
        context.wait_for(std::chrono::seconds(1));
        pbf_move_left_joystick(context, 128, 255, 40, 85);
        pbf_mash_button(context, BUTTON_L, 60);

        pbf_move_right_joystick(context, 128, 255, 250, 0);
        context.wait_for_all_requests();

        console.log("Finding center of platform...");
        if (!read_platform_center(platform_x, platform_y, m_env->program_info(), console)){
            console.log("Unable to find center of platform.", COLOR_RED);
            return;
        }
        console.log("Platform center at: x = " + tostr_default(platform_x) + ", y = " + tostr_default(platform_y), COLOR_BLUE);

        choose_path(console, move_x, move_y, duration, platform_x, platform_y);

        pbf_move_left_joystick(context, move_x, move_y, 40, 20);
        pbf_mash_button(context, BUTTON_L, 60);
//        pbf_wait(context, 1250);
    });
    clear_in_front(context, duration > 100, [&](BotBaseContext& context){
        console.log("Making location correction...");
        pbf_move_left_joystick(context, 128, 0, duration, 0);

        //  Optimization, calculate angle to aim you back at the sky.
        //  This speeds up the "find_and_center_on_sky()" call.
        double angle0 = std::atan2(move_x - 128., 128. - move_y);
        double angle1 = angle0 >= 0 ? 6.2831853071795864769 - angle0 : -6.2831853071795864769 - angle0;
        turn_angle(context, angle1);

        find_and_center_on_sky(*m_env, console, context);
        pbf_move_left_joystick(context, 96, 0, 40, 0);
        pbf_mash_button(context, BUTTON_L, 60);
    });

    //  One in every 4 iterations: Clear wall of spawns.
    if (m_iterations % 4 == 0){
        console.log("Turning along wall...");
        pbf_move_left_joystick(context, 0, 255, 20, 20);
        pbf_mash_button(context, BUTTON_L, 60);
        clear_in_front(context, true, [&](BotBaseContext& context){
            context.wait_for(std::chrono::milliseconds(1000));

            console.log("Turning back to sky.");
            pbf_move_left_joystick(context, 255, 255, 20, 20);
            pbf_mash_button(context, BUTTON_L, 60);
            find_and_center_on_sky(*m_env, console, context);
            pbf_move_left_joystick(context, 96, 0, 40, 0);
            pbf_mash_button(context, BUTTON_L, 60);
        });
    }

    if (platform_x < 0.5 || platform_y < 0.5){
        console.log("Not close enough to desired spot. Skipping forward attack...", COLOR_ORANGE);
        return;
    }

    clear_in_front(context, true, [&](BotBaseContext& context){
        console.log("Move forward, fire, and retreat.");
        switch (m_iterations % 3){
        case 0:
            pbf_move_left_joystick(context, 108, 0, 300, 0);
            break;
        case 1:
            pbf_move_left_joystick(context, 128, 0, 275, 0);
            break;
        case 2:
            pbf_move_left_joystick(context, 144, 0, 275, 0);
            break;
        }

    });
    clear_in_front(context, true, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 0);
        pbf_move_left_joystick(context, 128, 0, 60, 4 * TICKS_PER_SECOND);
    });

//    context.wait_for(std::chrono::seconds(60));
}
void ShinyHuntAreaZeroPlatform::run_traversal(BotBaseContext& context){
    switch (PATH0){
    case Path::PATH0:
        run_path0(context);
        break;
    case Path::PATH1:
        run_path1(context);
        break;
    case Path::PATH2:
        run_path2(context);
        break;
    }
    m_iterations++;
}


void ShinyHuntAreaZeroPlatform::on_shiny_encounter(
    BotBaseContext& context, EncounterWatcher& encounter_watcher
){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    stats.m_shinies++;
    m_env->update_stats();

    if (VIDEO_ON_SHINY){
        context.wait_for(std::chrono::seconds(3));
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    std::vector<std::pair<std::string, std::string>> embeds;
    embeds.emplace_back(
        "Detection Method:",
        "Shiny Sound (Error Coefficient = " + tostr_default(encounter_watcher.lowest_error_coefficient()) + ")"
    );
    send_program_notification(
        *m_env, NOTIFICATION_SHINY,
        COLOR_STAR_SHINY,
        "Found a Shiny!",
        std::move(embeds), "",
        encounter_watcher.shiny_screenshot(), true
    );

//    throw ProgramFinishedException();
}

void ShinyHuntAreaZeroPlatform::run_state(BotBaseContext& context){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = m_env->current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();
    const ProgramInfo& info = m_env->program_info();
    ConsoleHandle& console = m_env->console;
    WallClock now = current_time();

    send_program_status_notification(*m_env, NOTIFICATION_STATUS_UPDATE);

    State recovery_state = State::LEAVE_AND_RETURN;
    try{
        switch (m_state){
        case State::TRAVERSAL:{
            console.log("Run Traversal Iteration: " + tostr_u_commas(m_iterations));

            if (PLATFORM_RESET.enabled() && now - m_last_platform_reset > std::chrono::minutes(PLATFORM_RESET.RESET_DURATION_MINUTES)){
                m_state = State::LEAVE_AND_RETURN;
                return;
            }

            //  If we error out, recover using LEAVE_AND_RETURN.
            recovery_state = State::LEAVE_AND_RETURN;

            run_traversal(context);

            break;
        }
        case State::INSIDE_GATE_AND_RETURN:
            console.log("Going from inside gate to platform...");

            //  If we error out, recover using LEAVE_AND_RETURN.
            recovery_state = State::LEAVE_AND_RETURN;

            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);
            m_last_platform_reset = now;

            break;

        case State::LEAVE_AND_RETURN:
            console.log("Leaving and returning to platform...");

            //  If we error out, return to this state.
            recovery_state = State::LEAVE_AND_RETURN;

            return_to_inside_zero_gate(info, console, context);
            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);

            m_last_platform_reset = now;
            stats.m_platform_resets++;
            m_env->update_stats();

            break;

        case State::RESET_AND_RETURN:
            console.log("Resetting game and returning to platform...");

            //  If we error out, return to this state.
            recovery_state = State::RESET_AND_RETURN;

            m_last_platform_reset = current_time();
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_RCLICK, 20, 105);
            inside_zero_gate_to_platform(info, console, context, NAVIGATE_TO_PLATFORM);

            m_last_platform_reset = now;
            stats.m_game_resets++;
            m_env->update_stats();

            break;
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
            throw FatalProgramException(console, "Failed 3 times consecutively.");
        }
    }
}

void ShinyHuntAreaZeroPlatform::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    m_env = &env;

    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    assert_16_9_720p_min(env.logger(), env.console);

//    pbf_press_button(context, BUTTON_RCLICK, 20, 105);

    m_iterations = 0;

    LetsGoKillSoundDetector lets_go_kill_sound(
        env.console,
        [&](float){
            env.console.log("Detected kill.");
            stats.m_kills++;
            env.update_stats();
            return false;
        }
    );
    m_kill_watcher = &lets_go_kill_sound;
    InferenceSession session(
        context, env.console,
        {lets_go_kill_sound}
    );

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

    m_last_platform_reset = current_time();
    m_consecutive_failures = 0;
    while (true){
        env.console.log("Starting encounter loop...", COLOR_PURPLE);
        EncounterWatcher encounter_watcher(env.console, COLOR_RED);
        run_until(
            env.console, context,
            [&](BotBaseContext& context){
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
        stats.m_encounters++;
        m_env->update_stats();

        if (encounter_watcher.shiny_screenshot()){
            on_shiny_encounter(context, encounter_watcher);
            break;
        }

        OverworldWatcher overworld(COLOR_GREEN);
        run_until(
            env.console, context,
            [&](BotBaseContext& context){
                pbf_press_dpad(context, DPAD_DOWN, 250, 0);
                pbf_press_button(context, BUTTON_A, 20, 105);
                pbf_press_button(context, BUTTON_B, 20, 5 * TICKS_PER_SECOND);
            },
            {overworld}
        );
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
