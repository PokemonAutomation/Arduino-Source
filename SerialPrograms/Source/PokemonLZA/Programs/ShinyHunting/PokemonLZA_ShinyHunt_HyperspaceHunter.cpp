/*  Shiny Hunt - Hyperspace Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Programs/PokemonLZA_HyperspaceNavigation.h"
#include "PokemonLZA_ShinyHunt_HyperspaceHunter.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_HyperspaceHunter_Descriptor::ShinyHunt_HyperspaceHunter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-HyperspaceHunter", STRING_POKEMON + " LZA",
        "Hyperspace Hunter",
        "Programs/PokemonLZA/ShinyHunt-HyperspaceHunter.html",
        "Shiny hunt in Hyperspace Wild Zone using fly spot reset or shuttle run methods.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class ShinyHunt_HyperspaceHunter_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_HyperspaceHunter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_HyperspaceHunter::ShinyHunt_HyperspaceHunter()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , HUNT_METHOD("<b>Hunt Method:</b>",
        {
            {HuntMethod::FLY_SPOT_RESET, "fly_spot_reset", "Fly Spot Reset"},
            {HuntMethod::SHUTTLE_RUN, "shuttle_run", "Shuttle Run"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HuntMethod::FLY_SPOT_RESET
    )
    , NUM_RESETS(
        "<b>Max Hyperspace Resets:</b><br>Max number of resets when in Hyperspace. Set to zero to have no max reset limit. "
        "Make sure to leave enough time to catch found shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s. "
        "Each reset takes between 0.6-1.0 sec of the timer.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0 // default, min
    )
    , MIN_CALORIE_REMAINING(
        "<b>Minimum Cal. allowed While Resetting in Hyperspace:</b><br>The program will stop if the Calorie number is at or below this value."
        "<br>NOTE: the more star the hyperspace has the faster Calorie burns! Pick a minimum Calorie value that gives you enough time to catch shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s. "
        "Each reset takes between 0.6-1.0 sec of the timer."
        "<br>Use a donut with AT LEAST ONE flavor power for the program to read Calorie number correctly.",
        LockMode::UNLOCK_WHILE_RUNNING,
        120, 0, 9999 // default, min, max
    )
    // , PER_SPAWN_TABLE(
    //     "Custom Per Spawn Reset Command Table:",
    //     {
    //         ControllerClass::NintendoSwitch_ProController,
    //         ControllerClass::NintendoSwitch_LeftJoycon,
    //         ControllerClass::NintendoSwitch_RightJoycon,
    //     }
    // )
    // , PER_CHECK_TABLE(
    //     "Per Shiny Check Command Table:",
    //     {
    //         ControllerClass::NintendoSwitch_ProController,
    //         ControllerClass::NintendoSwitch_LeftJoycon,
    //         ControllerClass::NintendoSwitch_RightJoycon,
    //     }
    // )
    , FORWARD_RUN_TIME(
        "<b>Forward Run Time (Shuttle Run):</b><br>Duration to run forward in milliseconds.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , BACKWARD_RUN_TIME(
        "<b>Backward Run Time (Shuttle Run):</b><br>Duration to run backward in milliseconds.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "5500 ms"
    )
    , FORWARD_RUN_ROLL_TIME(
        "<b>Forward Run Roll Start (Shuttle Run):</b><br>Do a roll during forward running to cross a rooftop slope without losing speed. Set to 0 if no slope.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "0 ms"
    )
    , BACKWARD_RUN_ROLL_TIME(
        "<b>Backward Run Roll Start (Shuttle Run):</b><br>Do a roll during backward running to cross a rooftop slope without losing speed. Set to 0 if no slope.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "0 ms"
    )
    , SHINY_CHECK_RUN_TIME(
        "<b>Shiny Check Run Time (Shuttle Run):</b><br>After reaching reset limit (by Max Reset or Calorie Limit), "
        "run this long forward to listen for shiny sound.<br>If not detected, reset the game to do shuttle run again. Set to 0 to disable.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "5000 ms"
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
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(HUNT_METHOD);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(MIN_CALORIE_REMAINING);
    PA_ADD_OPTION(FORWARD_RUN_TIME);
    PA_ADD_OPTION(BACKWARD_RUN_TIME);
    PA_ADD_OPTION(FORWARD_RUN_ROLL_TIME);
    PA_ADD_OPTION(BACKWARD_RUN_ROLL_TIME);
    PA_ADD_OPTION(SHINY_CHECK_RUN_TIME);
    PA_ADD_OPTION(NOTIFICATIONS);
}

// Fly Spot Reset method - same as hyperspace_wild_zone route in FlySpotReset
void ShinyHunt_HyperspaceHunter::use_fly_spot_reset(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context)
{
    ShinyHunt_HyperspaceHunter_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HyperspaceHunter_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);
    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
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
            
            // Fly reset loop
            while (true){
                const bool zoom_to_max = false;
                const bool require_icons = false;
                if (!open_map(env.console, context, zoom_to_max, require_icons)){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "use_fly_spot_reset(): Cannot fast travel after being chased by wild pokemon.",
                        env.console
                    );
                }

                // Fly from map to reset spawns
                std::shared_ptr<const ImageRGB32> overworld_screen;
                FastTravelState travel_status = fly_from_map(env.console, context, &overworld_screen);
                if (travel_status != FastTravelState::SUCCESS){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "use_fly_spot_reset(): Cannot fast travel after moving map cursor.",
                        env.console
                    );
                }
                if (overworld_screen == nullptr){
                    throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "overworld_screen is nullptr but FastTravelState is successful.");
                }

                HyperspaceCalorieDetector hyperspace_calorie_detector(env.logger());
                if (!hyperspace_calorie_detector.detect(*overworld_screen)){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "use_fly_spot_reset(): Cannot read Calorie number on screen.",
                        env.console
                    );
                }

                const uint16_t calorie_number = hyperspace_calorie_detector.calorie_number();
                const uint16_t min_calorie = MIN_CALORIE_REMAINING;
                const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
                env.add_overlay_log(log_msg);
                env.log(log_msg);

                stats.resets++;
                env.update_stats();
                if (stats.resets % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }

                shiny_sound_handler.process_pending(context);

                if (calorie_number <= min_calorie){
                    env.log("min calorie reached");
                    env.add_overlay_log("Min Calorie Reached");
                    break;
                }
                if (NUM_RESETS > 0 && stats.resets >= NUM_RESETS){
                    env.log(std::format("Reached reset limit {}", static_cast<uint64_t>(NUM_RESETS)));
                    break;
                }
            } // end fly reset loop
        },
        {{shiny_detector}}
    );

    shiny_sound_handler.process_pending(context);
}


// Shuttle Run method - 
void ShinyHunt_HyperspaceHunter::use_shuttle_run(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context)
{

    ShinyHunt_HyperspaceHunter_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HyperspaceHunter_Descriptor::Stats>();

    // game reset loop
    while(true){
        size_t num_resets = 0;
        ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
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

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                // Shuttle run loop
                while(true){
                    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                    if (FORWARD_RUN_ROLL_TIME.get() == Milliseconds(0)){
                        pbf_move_left_joystick(context, {0, +1}, FORWARD_RUN_TIME, 0ms);
                    } else{
                        pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {0, 0}, FORWARD_RUN_ROLL_TIME);
                        pbf_controller_state(context, BUTTON_Y, DPAD_NONE, {0, +1}, {0, 0}, FORWARD_RUN_TIME.get() - FORWARD_RUN_ROLL_TIME.get());
                    }
                    if (BACKWARD_RUN_ROLL_TIME.get() == Milliseconds(0)){
                        pbf_move_left_joystick(context, {0, -1}, BACKWARD_RUN_TIME, 0ms);
                    } else{
                        pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, -1}, {0, 0}, BACKWARD_RUN_ROLL_TIME);
                        pbf_controller_state(context, BUTTON_Y, DPAD_NONE, {0, -1}, {0, 0}, BACKWARD_RUN_TIME.get() - BACKWARD_RUN_ROLL_TIME.get());
                    }
                    pbf_wait(context, 200ms);
                    context.wait_for_all_requests();

                    num_resets++;
                    stats.resets++;
                    env.update_stats();
                    if (num_resets % 10 == 0){
                        send_program_status_notification(env, NOTIFICATION_STATUS);
                    }
                    if (check_calorie(env.console, context, MIN_CALORIE_REMAINING)){
                        break;
                    }
                    if(NUM_RESETS > 0 && num_resets >= NUM_RESETS){
                        env.log(std::format("Reached reset limit {}", static_cast<uint64_t>(NUM_RESETS)));
                        break;
                    }
                } // end shuttle run

                // Run forward to check shiny
                if (SHINY_CHECK_RUN_TIME.get() > Milliseconds(0)){
                    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                    if (FORWARD_RUN_ROLL_TIME.get() == Milliseconds(0)){
                        pbf_move_left_joystick(context, {0, +1}, FORWARD_RUN_TIME.get() + SHINY_CHECK_RUN_TIME.get(), 0ms);
                    } else{
                        pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0, +1}, {0, 0}, FORWARD_RUN_ROLL_TIME);
                        const auto hold_time = FORWARD_RUN_TIME.get() + SHINY_CHECK_RUN_TIME.get() - FORWARD_RUN_ROLL_TIME.get();
                        pbf_controller_state(context, BUTTON_Y, DPAD_NONE, {0, +1}, {0, 0}, hold_time);
                    }
                    // wait some time to listen to shiny sound
                    pbf_wait(context, 2000ms);
                    context.wait_for_all_requests();
                }
            },
            {{shiny_detector}}
        );

        shiny_sound_handler.process_pending(context);
        if (ret == 0){
            // there is shiny sound and user requested to stop program when shiny sound:
            return;
        }
        
        // Reset game
        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);
    } // end game reset loop
}


void ShinyHunt_HyperspaceHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (HUNT_METHOD == HuntMethod::SHUTTLE_RUN){
        if (FORWARD_RUN_ROLL_TIME.get() > Milliseconds(0) && FORWARD_RUN_ROLL_TIME.get() > FORWARD_RUN_TIME.get()){
            throw UserSetupError(env.console, "Forward Run Roll Start time must not be greater than Forward Run time.");
        }
        if (BACKWARD_RUN_ROLL_TIME.get() > Milliseconds(0) && BACKWARD_RUN_ROLL_TIME.get() > BACKWARD_RUN_TIME.get()){
            throw UserSetupError(env.console, "Backward Run Roll Start time must not be greater than Backward Run time.");
        }
    }
    assert_16_9_720p_min(env.logger(), env.console);
    HyperspaceCalorieDetector::warm_ocr();

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    if (HUNT_METHOD == HuntMethod::FLY_SPOT_RESET){
        use_fly_spot_reset(env, context);
    } else if (HUNT_METHOD == HuntMethod::SHUTTLE_RUN){
        use_shuttle_run(env, context);
    }
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    go_home(env.console, context);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
