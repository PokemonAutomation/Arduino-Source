/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_ShinyHunt-AreaZeroPlatform.h"


//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



NavigatePlatformSettings::NavigatePlatformSettings()
    : GroupOption("Navigate to Platform Settings", LockWhileRunning::UNLOCKED)
    , STATION_ARRIVE_PAUSE_SECONDS(
        "<b>Station Arrive Pause</b><br>Pause for this many seconds after leaving the station. "
        "This allows stuff to load to reduce the chance of lag affecting the fly to platform.",
        LockWhileRunning::UNLOCKED,
        1
    )
    , MIDAIR_PAUSE_TIME(
        "<b>Mid-Air Pause Time:</b><br>Pause for this long before final approach to the platform. "
        "Too small and you may crash into the wall or have reduced spawns. "
        "Too large and you may undershoot the platform.",
        LockWhileRunning::UNLOCKED,
        TICKS_PER_SECOND,
        "50"
    )
{
    PA_ADD_OPTION(STATION_ARRIVE_PAUSE_SECONDS);
    PA_ADD_OPTION(MIDAIR_PAUSE_TIME);
}




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
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Kills");
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& m_kills;
    std::atomic<uint64_t>& m_encounters;
    std::atomic<uint64_t>& m_shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntAreaZeroPlatform_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




ShinyHuntAreaZeroPlatform::ShinyHuntAreaZeroPlatform()
    : MODE(
        "<b>Mode:</b><br>"
        "If the mode requires starting on the platform, you should stand near the center of the platform facing any direction.<br><br>"
        "If the mode requires starting in the Zero Gate, you should be just inside the building as if you just entered.<br><br>"
        "If the mode periodically resets, you must have saved in your starting position.<br><br>"
        "Over time, the number of spawns decreases as they get stuck in unknown places. "
        "This option lets you periodically reset the game to refresh everything. "
        "However, resetting means you lose any shinies that have spawned, but "
        "not yet encountered.",
        {
            {Mode::START_ON_PLATFORM_NO_RESET,          "platform-no-reset",        "Start on platform. Do not reset."},
            {Mode::START_IN_ZERO_GATE_NO_RESET,         "zerogate-no-reset",        "Start inside Zero Gate. Do not reset."},
            {Mode::START_IN_ZERO_GATE_PERIODIC_RESET,   "zerogate-periodic-reset",  "Start inside Zero Gate. Periodically reset."},
        },
        LockWhileRunning::LOCKED,
        Mode::START_ON_PLATFORM_NO_RESET
    )
    , RESET_DURATION_MINUTES(
        "<b>Reset Duration (minutes):</b><br>If you are resetting, reset the game every "
        "this many minutes.",
        LockWhileRunning::UNLOCKED,
        180
    )
    , PATH(
        "<b>Path:</b><br>Traversal path on the platform to trigger encounters.",
        {
            {Path::PATH0, "path0", "Path 0"},
            {Path::PATH1, "path1", "Path 1"},
        },
        LockWhileRunning::UNLOCKED,
        Path::PATH0
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
        PA_ADD_OPTION(RESET_DURATION_MINUTES);
    }
    PA_ADD_OPTION(PATH);
    PA_ADD_OPTION(VIDEO_ON_SHINY);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(NAVIGATE_TO_PLATFORM);
    }
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void ShinyHuntAreaZeroPlatform::zero_gate_to_platform(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context
){
    zero_gate_to_station(info, console, context, 2);

    context.wait_for(std::chrono::seconds(NAVIGATE_TO_PLATFORM.STATION_ARRIVE_PAUSE_SECONDS));

    //  Navigate to platform.
#if 0
    //  Don't jump to avoid spawn.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 625, 0);
    ssf_press_button(context, BUTTON_B, 0, 50);
    pbf_move_left_joystick(context, 128, 0, 250, 0);
    pbf_move_left_joystick(context, 160, 0, 600, 0);
    pbf_move_left_joystick(context, 128, 0, 1875, 0);
#endif

#if 0
    //  Jump late.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);

    ssf_press_joystick(context, true, 128, 0, 315, 500);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 1150, 0);
    pbf_move_left_joystick(context, 128, 0, 125, NAVIGATE_TO_PLATFORM.MIDAIR_PAUSE_TIME);

    pbf_move_left_joystick(context, 128, 0, 1375, 250);
#endif

#if 0
    //  Jump earlier.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);

    ssf_press_joystick(context, true, 128, 0, 280, 500);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 1150, 0);
    pbf_move_left_joystick(context, 128, 0, 125, NAVIGATE_TO_PLATFORM.MIDAIR_PAUSE_TIME);

    pbf_move_left_joystick(context, 128, 0, 1375, 250);
#endif

#if 1
    //  Jump on the downhill to improve chance of clearing things.
    pbf_move_left_joystick(context, 192, 0, 20, 105);
    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_joystick(context, true, 128, 0, 125, 1250);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 700, 0);
    pbf_move_left_joystick(context, 128, 0, 125, NAVIGATE_TO_PLATFORM.MIDAIR_PAUSE_TIME);
    pbf_move_left_joystick(context, 128, 0, 875, 250);
#endif

//    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 5 * TICKS_PER_SECOND, 0);
}




enum class OverworldState{
    None,
    FindingSky,
    TurningLeft,
    TurningRight,
};
void find_and_center_on_sky(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    console.log("Looking for the sky...");

    AreaZeroSkyTracker sky_tracker(console);
    InferenceSession inference_session(
        context, console,
        {sky_tracker}
    );

    AsyncCommandSession session(context, console, env.realtime_dispatcher(), context.botbase());
    OverworldState state = OverworldState::None;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(1)){
            throw OperationFailedException(console, "Failed to find the sky after 1 minute.", true);
        }

        context.wait_for(std::chrono::milliseconds(200));

        double sky_x, sky_y;
        bool sky = sky_tracker.sky_location(sky_x, sky_y);

        if (!sky){
            if (!session.command_is_running() || state != OverworldState::FindingSky){
                console.log("Sky not detected. Attempting to find the sky...", COLOR_ORANGE);
                session.dispatch([](BotBaseContext& context){
                    pbf_move_right_joystick(context, 128, 0, 250, 0);
                    pbf_move_right_joystick(context, 0, 0, 10 * TICKS_PER_SECOND, 0);
                });
                state = OverworldState::FindingSky;
            }
            continue;
        }

//        cout << sky_x << " - " << sky_y << endl;

        if (sky_x < 0.45){
            if (!session.command_is_running() || state != OverworldState::TurningLeft){
                console.log("Centering the sky... Moving left.", COLOR_BLUE);
                uint8_t magnitude = (uint8_t)((0.5 - sky_x) * 96 + 31);
                uint16_t duration = (uint16_t)((0.5 - sky_x) * 125 + 20);
                session.dispatch([=](BotBaseContext& context){
                    pbf_move_right_joystick(context, 128 - magnitude, 128, duration, 0);
                });
                state = OverworldState::TurningLeft;
            }
            continue;
        }
        if (sky_x > 0.55){
            if (!session.command_is_running() || state != OverworldState::TurningRight){
                console.log("Centering the sky... Moving Right.", COLOR_BLUE);
                uint8_t magnitude = (uint8_t)((sky_x - 0.5) * 96 + 31);
                uint16_t duration = (uint16_t)((sky_x - 0.5) * 125 + 20);
                session.dispatch([=](BotBaseContext& context){
                    pbf_move_right_joystick(context, 128 + magnitude, 128, duration, 0);
                });
                state = OverworldState::TurningRight;
            }
            continue;
        }

        if (session.command_is_running()){
            session.stop_command();
            state = OverworldState::None;
            context.wait_for(std::chrono::seconds(1));
            continue;
        }
        break;
    }

    session.stop_session_and_rethrow();
}

bool ShinyHuntAreaZeroPlatform::clear_in_front(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    std::function<void(BotBaseContext& context)>&& command
){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

//    static int calls = 0;
    console.log("Clearing what's in front with Let's Go...");
//    cout << calls++ << endl;
    pbf_press_button(context, BUTTON_R, 20, 0);
    LetsGoKillWatcher kill_watcher(
        console, COLOR_YELLOW, false,
        [&](){
            stats.m_kills++;
            env.update_stats();
        }
    );
    WallClock last_kill = kill_watcher.last_kill();
    context.wait_for_all_requests();
    std::chrono::seconds timeout(6);
    while (true){
        if (command){
//            cout << "running command..." << endl;
            run_until(
                console, context,
                std::move(command),
                {kill_watcher}
            );
            command = nullptr;
        }else{
//            cout << "Waiting out... " << timeout.count() << " seconds" << endl;
            wait_until(
                console, context,
                timeout,
                {kill_watcher}
            );
        }
        timeout = std::chrono::seconds(3);
        if (last_kill == kill_watcher.last_kill()){
//            cout << "no kill" << endl;
            break;
        }
//        cout << "found kill" << endl;
        last_kill = kill_watcher.last_kill();
    }
    console.log("Nothing left to clear...");
    return kill_watcher.last_kill() != WallClock::min();
}

void ShinyHuntAreaZeroPlatform::run_path0(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    //  Go back to the wall.
    console.log("Go back to wall...");
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        find_and_center_on_sky(env, console, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 176, 255, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    clear_in_front(env, console, context, [&](BotBaseContext& context){
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
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        find_and_center_on_sky(env, console, context);
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
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
    });
}
void ShinyHuntAreaZeroPlatform::run_path1(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    //  Go back to the wall.
    console.log("Go back to wall...");
    pbf_press_button(context, BUTTON_L, 20, 50);
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        find_and_center_on_sky(env, console, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 192, 255, 60, 0);
    });

    //  Clear path to the wall.
    console.log("Clear path to the wall...");
    pbf_press_button(context, BUTTON_L, 20, 50);
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 128, 0, 5 * TICKS_PER_SECOND, 0);

        //  Turn right.
        pbf_move_left_joystick(context, 255, 128, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    //  Clear the wall.
    console.log("Clear the wall...");
    uint16_t duration = 325;
    clear_in_front(env, console, context, [&](BotBaseContext& context){
        pbf_move_left_joystick(context, 255, 128, 125, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
        context.wait_for_all_requests();

        find_and_center_on_sky(env, console, context);
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
    clear_in_front(env, console, context, [&](BotBaseContext& context){
//        pbf_move_left_joystick(context, 64, 0, 125, 0);
//        pbf_press_button(context, BUTTON_L, 20, 105);
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
//        pbf_controller_state(context, 0, DPAD_NONE, 255, 255, 120, 128, 3 * TICKS_PER_SECOND);
    });
}
void ShinyHuntAreaZeroPlatform::run_iteration(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context
){
    switch (PATH){
    case Path::PATH0:
        run_path0(env, console, context);
        break;
    case Path::PATH1:
        run_path1(env, console, context);
        break;
    }
}




void ShinyHuntAreaZeroPlatform::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ShinyHuntAreaZeroPlatform_Descriptor::Stats& stats = env.current_stats<ShinyHuntAreaZeroPlatform_Descriptor::Stats>();

    assert_16_9_720p_min(env.logger(), env.console);

    if (MODE == Mode::START_IN_ZERO_GATE_NO_RESET || MODE == Mode::START_IN_ZERO_GATE_PERIODIC_RESET){
        zero_gate_to_platform(env.program_info(), env.console, context);
    }

    WallClock last_reset = current_time();
    std::unique_ptr<EncounterWatcher> encounter_watcher;
    encounter_watcher.reset(new EncounterWatcher(env.console, COLOR_RED));
    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        m_iterations++;

        WallClock now = current_time();
        if (MODE == Mode::START_IN_ZERO_GATE_PERIODIC_RESET && now - last_reset > std::chrono::minutes(RESET_DURATION_MINUTES)){
            last_reset = now;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env.program_info(), env.console, context, 5 * TICKS_PER_SECOND);
            zero_gate_to_platform(env.program_info(), env.console, context);
        }

        context.wait_for_all_requests();
        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                run_iteration(env, env.console, context);
            },
            {
                static_cast<VisualInferenceCallback&>(*encounter_watcher),
                static_cast<AudioInferenceCallback&>(*encounter_watcher),
            }
        );
        encounter_watcher->throw_if_no_sound();
        if (ret != 0){
            continue;
        }

        env.console.log("Detected battle.", COLOR_PURPLE);
        stats.m_encounters++;
        env.update_stats();

        if (encounter_watcher->shiny_screenshot()){
            stats.m_shinies++;
            env.update_stats();

            if (VIDEO_ON_SHINY){
                context.wait_for(std::chrono::seconds(5));
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
            }

            std::vector<std::pair<std::string, std::string>> embeds;
            embeds.emplace_back(
                "Detection Method:",
                "Shiny Sound (Error Coefficient = " + tostr_default(encounter_watcher->lowest_error_coefficient()) + ")"
            );
            send_program_notification(
                env, NOTIFICATION_SHINY,
                COLOR_STAR_SHINY,
                "Found a Shiny!",
                std::move(embeds), "",
                encounter_watcher->shiny_screenshot(), true
            );

            GO_HOME_WHEN_DONE.run_end_of_program(context);

            throw ProgramFinishedException();
        }

        //  Clear the detection history to prepare next encounter.
        encounter_watcher.reset(new EncounterWatcher(env.console, COLOR_RED));

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

}




}
}
}
