/*  Shiny Hunt - Hyperspace Legendary
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
#include "PokemonLZA_ShinyHunt_HyperspaceLegendary.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_HyperspaceLegendary_Descriptor::ShinyHunt_HyperspaceLegendary_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-HyperspaceLegendary", STRING_POKEMON + " LZA",
        "Hyperspace Legendary",
        "Programs/PokemonLZA/ShinyHunt-HyperspaceLegendary.html",
        "Shiny hunt legendary " + STRING_POKEMON + " in Hyperspace.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class ShinyHunt_HyperspaceLegendary_Descriptor::Stats : public StatsTracker{
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

std::unique_ptr<StatsTracker> ShinyHunt_HyperspaceLegendary_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_HyperspaceLegendary::ShinyHunt_HyperspaceLegendary()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , LEGENDARY("<b>Hunt Route:</b>",
        {
            {Legendary::VIRIZION,  "virizion",  "Virizion"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Legendary::VIRIZION
    )
    , MAX_ROUNDS(
        "<b>Max Rounds:</b><br>Max number of spawn attempts. Set to zero to have no max round limit. "
        "Make sure to leave enough time to catch found shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0 // default, min
    )
    , MIN_CALORIE_REMAINING(
        "<b>Minimum Cal. allowed:</b><br>The program will stop if the Calorie number is at or below this value."
        "<br>NOTE: the more star the hyperspace has the faster Calorie burns! Pick a minimum Calorie value that gives you enough time to catch shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s",
        LockMode::UNLOCK_WHILE_RUNNING,
        600, 0, 9999 // default, min, max
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
    PA_ADD_OPTION(LEGENDARY);
    PA_ADD_OPTION(MAX_ROUNDS);
    PA_ADD_OPTION(MIN_CALORIE_REMAINING);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

// Return if the loop should stop
typedef std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&, ShinyHunt_HyperspaceLegendary_Descriptor::Stats&, bool)> route_func;

void route_default(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats){
    // Open map
    bool can_fast_travel = open_map(env.console, context);
    if (!can_fast_travel){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_default(): Cannot open map for fast travel.",
            env.console
        );
    }

    // Move map cursor upwards a little bit
    pbf_move_left_joystick(context, {0, +0.5}, 100ms, 200ms);

    // Fly from map to reset spawns
    FastTravelState travel_status = fly_from_map(env.console, context);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_default(): Cannot fast travel after moving map cursor.",
            env.console
        );
    }
}

bool route_virizion(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_REMAINING,
    SimpleIntegerOption<uint16_t> MAX_ROUNDS,
    uint8_t& ready_to_stop_counter){

    const uint16_t min_calorie = MIN_CALORIE_REMAINING;

    // running forward
    //Milliseconds duration(4400);

    HyperspaceCalorieLimitWatcher calorie_watcher(env.logger(), min_calorie);
    const int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            // running forward
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 536ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0.000, 1.000}, {1.000, -0.000}, 173ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 348ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0.000, 1.000}, {-1.000, -0.000}, 38ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 572ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 716ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 385ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0.000, 1.000}, {-1.000, -0.000}, 123ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 178ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 212ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 1634ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0.000, 1.000}, {-1.000, -0.000}, 91ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 994ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 221ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 844ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 155ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 81ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 116ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 67ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 111ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 78ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 111ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 72ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 112ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 78ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 134ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 78ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 132ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 71ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 132ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 68ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 134ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 66ms, 0ms);
            pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 131ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 1925ms, 0ms);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.000, 1.000}, {0.000, -0.000}, 174ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 1321ms, 0ms);
            pbf_controller_state(context, BUTTON_NONE, DPAD_NONE, {0.000, 1.000}, {0.000, -1.000}, 1389ms);
            pbf_move_left_joystick(context, {0.000000, 1.000000}, 196ms, 0ms);
            pbf_wait(context, 3337ms);
        },
        {{calorie_watcher}}
    );
    uint16_t calorie_number = calorie_watcher.calorie_number();
    const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
    env.add_overlay_log(log_msg);
    env.log(log_msg);
    if (ret == 0){
        env.log("min calorie reached");
        return true;
    }

    return true;
}


} // namespace

void ShinyHunt_HyperspaceLegendary::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 200ms);

    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HyperspaceLegendary_Descriptor::Stats>();

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

    uint64_t num_resets = 0;
    uint8_t ready_to_stop_counter = 0;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);

                bool should_reset = route_virizion(
                    env, context, stats, MIN_CALORIE_REMAINING, ready_to_stop_counter
                    );

                if (!should_reset){
                    break;
                }

                go_home(env.console, context);
                reset_game_from_home(env, env.console, context);

                num_resets++;
                stats.resets++;
                env.update_stats();

                if (num_resets % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }

                if (MAX_ROUNDS > 0 && num_resets >= MAX_ROUNDS){
                    env.log("Reached reset limit.");
                    break;
                }
            }
        },
        {{shiny_detector}}
        );

    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    go_home(env.console, context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation

