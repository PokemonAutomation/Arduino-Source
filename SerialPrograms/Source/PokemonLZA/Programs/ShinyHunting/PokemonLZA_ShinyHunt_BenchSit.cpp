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
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
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
        STRING_POKEMON + " LZA", "Bench Sit",
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
    , PERIODIC_SAVE(
        "<b>Periodically Save:</b><br>"
        "Save the game every this many bench sits. This reduces the loss to game crashes. Set to zero to disable. Saving will be unsuccessful if you are under attack",
        LockMode::UNLOCK_WHILE_RUNNING,
        100,
        0
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
    PA_ADD_OPTION(PERIODIC_SAVE);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void run_back_until_found_bench(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    ButtonWatcher buttonA(
        COLOR_RED,
        ButtonType::ButtonA,
        {0.486, 0.477, 0.115, 0.5},
        &env.console.overlay()
    );

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 800ms, 0ms);
            pbf_move_left_joystick(context, {0, -1}, 800ms, 200ms);
            pbf_press_button(context, BUTTON_L, 160ms, 160ms);

            //  Can't just hold it down since sometimes it doesn't register.
            for (int c = 0; c < 10; c++){
                pbf_move_right_joystick(context, {-1, 0}, 800ms, 200ms);
                pbf_press_button(context, BUTTON_L, 160ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 840ms, 800ms);
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
            "run_back_until_found_bench(): Unable to detect bench after multiple attempts.",
            env.console
        );
    }
}

void ShinyHunt_BenchSit::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    ShinyHunt_BenchSit_Descriptor::Stats& stats = env.current_stats<ShinyHunt_BenchSit_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        env.update_stats();
        env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
        return shiny_sound_handler.on_shiny_sound(
            env, env.console,
            stats.shinies,
            error_coefficient
        );
    });

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (uint32_t rounds_since_last_save = 0;; rounds_since_last_save++) {
                send_program_status_notification(env, NOTIFICATION_STATUS);
                sit_on_bench(env.console, context);
                shiny_sound_handler.process_pending(context);
                stats.resets++;
                env.update_stats();

                uint32_t periodic_save = PERIODIC_SAVE;
                if (periodic_save != 0 && rounds_since_last_save >= periodic_save) {
                    bool save_successful = save_game_to_menu(env.console, context);
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                    if (save_successful) {
                        env.console.overlay().add_log("Game Saved Successfully", COLOR_BLUE);
                        rounds_since_last_save = 0;
                    } else {
                        env.console.overlay().add_log("Game Save Failed. Will attempt to save after the next reset.", COLOR_RED);
                    }
                }

                Milliseconds duration = WALK_FORWARD_DURATION;
                if (duration > Milliseconds::zero()){
                    if (WALK_DIRECTION.current_value() == 0){ // forward
                        env.console.overlay().add_log("Move Forward");
                        ssf_press_button(context, BUTTON_B, 0ms, 2*duration, 0ms);
                        pbf_move_left_joystick(context, 128, 0, duration, 0ms);
                        // run back
                        pbf_move_left_joystick(context, {0, -1}, duration + 750ms, 0ms);
                        run_back_until_found_bench(env, context);
                    }else if (WALK_DIRECTION.current_value() == 1){ // left
                        env.console.overlay().add_log("Move Left");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {-1, 0},  duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 100ms, 400ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {0, -1}, duration, 0ms);
                        pbf_move_left_joystick(context, {-1, 0},  100ms, 0ms);
                    }else if (WALK_DIRECTION.current_value() == 2){ // right
                        env.console.overlay().add_log("Move Right");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, 255, 128, duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 100ms, 400ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {0, -1}, duration, 0ms);
                        pbf_move_left_joystick(context, 255, 128, 100ms, 0ms);
                    }
                }else{
                    run_back_until_found_bench(env, context);
                }

                shiny_sound_handler.process_pending(context);
            }
        },
        {shiny_detector}
    );

    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}














}
}
}
