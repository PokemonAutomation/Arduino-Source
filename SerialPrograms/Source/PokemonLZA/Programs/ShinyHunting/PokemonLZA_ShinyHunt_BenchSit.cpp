/*  Shiny Hunt - Bench Sit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_BenchSit.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_BenchSit_Descriptor::ShinyHunt_BenchSit_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-BenchSit",
        STRING_POKEMON + " LZA", "Shiny Hunt - Bench Sit",
        "Programs/PokemonLZA/ShinyHunt-BenchSit.html",
        "Shiny hunt by repeatedly sitting on a bench to reset spawns.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class ShinyHunt_BenchSit_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Bench Sits"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Bench Sits");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Shinies"] = "Shiny Sounds";
        m_aliases["Shinies Detected"] = "Shiny Sounds";
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_BenchSit_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_BenchSit::ShinyHunt_BenchSit()
    : WALK_DIRECTION(
        "<b>Run Direction:</b><br>The direction of running after each day change to increase the spawn radius.",
        {
            {0, "forward", "Forward"},
            {1, "left", "Turn Left"},
            {2, "right", "Turn Right"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , WALK_FORWARD_DURATION(
        "<b>Run Forward Duration</b><br>"
        "Run forward and backward for this long after each day change to "
        "increase the spawn radius. Set to zero to disable this.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , SHINY_DETECTED(
        "Shiny Detected", "",
        "2000 ms",
        ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(WALK_DIRECTION);
    }
    PA_ADD_OPTION(WALK_FORWARD_DURATION);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void reapproach_bench_after_getting_up(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ButtonWatcher buttonA(
        COLOR_RED,
        ButtonType::ButtonA,
        {0.4, 0.3, 0.2, 0.7},
        &env.console.overlay()
    );

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            //  Can't just hold it down since sometimes it doesn't register.
            for (int c = 0; c < 60; c++){
                pbf_move_left_joystick(context, 128, 255, 800ms, 200ms);
            }
        },
        {buttonA}
    );

    switch (ret){
    case 0:
        env.console.log("Detected floating A button...");
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "sit_on_bench(): Unable to detect bench after 60 seconds.",
            env.console
        );
    }
}

void ShinyHunt_BenchSit::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_BenchSit_Descriptor::Stats& stats = env.current_stats<ShinyHunt_BenchSit_Descriptor::Stats>();

    std::atomic<uint8_t> to_take_shiny_sound_video{0};
    // Store shiny detection time as milliseconds since epoch for thread-safe access
    std::atomic<int64_t> shiny_detection_time_ms{0};

    // once shiny sound detector finds a shiny and user decides to take a video of it,
    // we need to take the video on Switch as part of the program loop.
    auto handle_shiny_sound_video_request = [&]() -> void {
        if (to_take_shiny_sound_video.load(std::memory_order_relaxed) && SHINY_DETECTED.TAKE_VIDEO){
            // Calculate elapsed time since shiny detection
            int64_t detection_time_ms = shiny_detection_time_ms.load(std::memory_order_relaxed);
            WallDuration elapsed = current_time() - WallClock(Milliseconds(detection_time_ms));
            auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);

            // Calculate remaining time to wait
            Milliseconds requested_delay = SHINY_DETECTED.SCREENSHOT_DELAY.get();
            if (requested_delay > elapsed_ms){
                context.wait_for(requested_delay - elapsed_ms);
            }
            // Otherwise, take screenshot immediately (no additional wait needed)

            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
            env.console.overlay().add_log("Take Video");
            to_take_shiny_sound_video.store(0, std::memory_order_relaxed);
        }
    };

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        env.update_stats();

        if ((SHINY_DETECTED.ACTION == ShinySoundDetectedAction::STOP_PROGRAM) || stats.shinies == 1){
            // Record the detection time for video delay calculation
            WallClock now = current_time();
            int64_t now_ms = std::chrono::duration_cast<Milliseconds>(now.time_since_epoch()).count();
            shiny_detection_time_ms.store(now_ms, std::memory_order_relaxed);

            SHINY_DETECTED.send_shiny_sound_notification(env, env.console, error_coefficient);
            env.console.overlay().add_log("Shiny sound detected!", COLOR_YELLOW);
            to_take_shiny_sound_video.store(1, std::memory_order_relaxed);
        }

        return SHINY_DETECTED.ACTION == ShinySoundDetectedAction::STOP_PROGRAM;
    });

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                send_program_status_notification(env, NOTIFICATION_STATUS);
                sit_on_bench(env.console, context);
                handle_shiny_sound_video_request();
                stats.resets++;
                env.update_stats();
                Milliseconds duration = WALK_FORWARD_DURATION;
                if (duration > Milliseconds::zero()){
                    if (WALK_DIRECTION.current_value() == 0){ // forward
                        env.console.overlay().add_log("Move Forward");
                        ssf_press_button(context, BUTTON_B, 0ms, 2 * duration, 0ms);
                        pbf_move_left_joystick(context, 128, 0, duration, 0ms);
                        pbf_move_left_joystick(context, 128, 255, duration + 500ms, 0ms);
                    } else if (WALK_DIRECTION.current_value() == 1){ // left
                        env.console.overlay().add_log("Move Left");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, 0, 128, duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 100ms, 400ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, 128, 255, duration, 0ms);
                        pbf_move_left_joystick(context, 0, 128, 100ms, 0ms);
                    } else if (WALK_DIRECTION.current_value() == 2){ // right
                        env.console.overlay().add_log("Move Right");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, 255, 128, duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 100ms, 400ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, 128, 255, duration, 0ms);
                        pbf_move_left_joystick(context, 255, 128, 100ms, 0ms);
                    }
                }
                else{
                    reapproach_bench_after_getting_up(env, context);
                }

                handle_shiny_sound_video_request();
            }
        },
        {{shiny_detector}}
    );

    // Shiny sound detected and user requested stopping the program when detected shiny sound

    handle_shiny_sound_video_request();

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}














}
}
}
