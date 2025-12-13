/*  Shiny Hunt - Fly Spot Reset
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
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_FlySpotReset.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_FlySpotReset_Descriptor::ShinyHunt_FlySpotReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-FlySpotReset", STRING_POKEMON + " LZA",
        "Fly Spot Reset",
        "Programs/PokemonLZA/ShinyHunt-FlySpotReset.html",
        "Shiny hunt by repeatedly fast traveling to a location to reset nearby " + STRING_POKEMON + " spawns.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class ShinyHunt_FlySpotReset_Descriptor::Stats : public StatsTracker{
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

std::unique_ptr<StatsTracker> ShinyHunt_FlySpotReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_FlySpotReset::ShinyHunt_FlySpotReset()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , ROUTE("<b>Hunt Route:</b>",
        {
            {Route::NO_MOVEMENT,  "no_movement",  "No Movement in Lumiose"},
            {Route::HYPERSPACE_WILD_ZONE, "hyperspace_wild_zone", "Hyperspace Wild Zone"},
            {Route::WILD_ZONE_19, "wild_zone_19", "Wild Zone 19"},
            {Route::ALPHA_PIDGEY, "alpha_pidgey", "Alpha Pidgey (Wild Zone 1)"},
            // {Route::ALPHA_PIKACHU, "alpha_pikachu", "Alpha Pikachu (Wild Zone 6)"},
            // {Route::CUSTOMISED_MACRO, "customised_macro", "Customised Macro"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Route::NO_MOVEMENT
    )
    , NUM_RESETS(
        "<b>Hyperspace Resets:</b><br>Number of resets when running the Hyperspace Wild Zone route. Make sure to leave enough time to catch found shinies."
        "<br>Approximate Star/Cal. per reset: 1 Star/1 Cal, 2 Star/1-1.5 Cal., 3 Star/2-2.5 Cal., 4 Star/5 Cal., 5 Star/6 Cal.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 1
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
    PA_ADD_OPTION(ROUTE);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

typedef std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&, ShinyHunt_FlySpotReset_Descriptor::Stats&, bool)> route_func;

void route_default(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max){
    // Open map
    bool can_fast_travel = open_map(env.console, context, to_zoom_to_max);
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
    pbf_move_left_joystick(context, 128, 64, 100ms, 200ms);

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

void route_hyperspace_wild_zone(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max){
    open_hyperspace_map(env.console, context);
    
    // Fly from map to reset spawns
    FastTravelState travel_status = fly_from_map(env.console, context);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_hyperspace_wild_zone(): Cannot fast travel after moving map cursor.",
            env.console
        );
    }
}

void route_wild_zone_19(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max){
    if (run_a_straight_path_in_overworld(env.console, context, 0, 80, 6500ms) == 0) {
        open_map(env.console, context, to_zoom_to_max);
        pbf_move_left_joystick(context, 0, 64, 100ms, 100ms);
        if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
            pbf_move_left_joystick(context, 128, 192, 100ms, 100ms);
            fly_from_map(env.console, context);
        }
    } else {
        open_map(env.console, context, to_zoom_to_max);
        pbf_move_left_joystick(context, 0, 64, 100ms, 100ms);
        fly_from_map(env.console, context);
    }
    wait_until_overworld(env.console, context, 50s);
}

void route_alpha_pidgey(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max){
    int ret = -1;
    {
        BlackScreenOverWatcher black_screen(COLOR_BLUE);
        ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                pbf_move_left_joystick(context, 255, 128, 4000ms, 0ms);
                pbf_move_left_joystick(context, 128, 255, 7400ms, 0ms);
                pbf_move_left_joystick(context, 0, 128, 3000ms, 0ms);
                pbf_press_button(context, BUTTON_A, 500ms, 2500ms); // elevator up
                pbf_move_left_joystick(context, 255, 128, 100ms, 0ms);
                pbf_press_button(context, BUTTON_L, 100ms, 1000ms);
            },
            {black_screen}
        );
    }
    if (ret == 0){
        wait_until_overworld(env.console, context, 50s);
    }
    open_map(env.console, context, to_zoom_to_max);
    pbf_move_left_joystick(context, 128, 255, 200ms, 100ms);
    if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
        pbf_move_left_joystick(context, 255, 128, 100ms, 100ms);
        fly_from_map(env.console, context);
    }
    wait_until_overworld(env.console, context);
}

} // namespace

void ShinyHunt_FlySpotReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    ShinyHunt_FlySpotReset_Descriptor::Stats& stats = env.current_stats<ShinyHunt_FlySpotReset_Descriptor::Stats>();

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

    route_func route;
    switch (ROUTE) {
    case Route::NO_MOVEMENT:
        route = route_default;
        break;
    case Route::HYPERSPACE_WILD_ZONE:
        route = route_hyperspace_wild_zone;
        break;
    case Route::WILD_ZONE_19:
        route = route_wild_zone_19;
        break;
    case Route::ALPHA_PIDGEY:
        route = route_alpha_pidgey;
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route not implemented",
            env.console
        );
    }
    
    uint64_t num_resets = 0;
    bool to_zoom_to_max = true;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);
                route(env, context, stats, to_zoom_to_max);
                to_zoom_to_max = false;
                num_resets++;
                stats.resets++;
                env.update_stats();
                if (stats.resets.load(std::memory_order_relaxed) % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }

                uint64_t num_resets_temp = NUM_RESETS;
                if (ROUTE == Route::HYPERSPACE_WILD_ZONE && num_resets >= num_resets_temp){
                    env.log("Number of resets hit. Going to home to pause the game.");
                    go_home(env.console, context);
                    break;
                }
            } // end while
        },
        {{shiny_detector}}
    );

    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
