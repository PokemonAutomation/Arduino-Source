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
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
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
        AllowCommandsWhenRunning::DISABLE_COMMANDS
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
    PA_ADD_OPTION(MIN_CALORIE_REMAINING);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

// Return if the loop should stop
using route_func = std::function<
    void(
        SingleSwitchProgramEnvironment&,
        ProControllerContext&,
        ShinyHunt_FlySpotReset_Descriptor::Stats&,
        bool
    )
>;

void route_default(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max
){
    // Open map
    bool can_fast_travel = open_map(env.console, context, to_zoom_to_max, false);
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

void route_wild_zone_19(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max){
    if (run_a_straight_path_in_overworld(env.console, context, -1, 0.375, 6500ms) == 0) {
        open_map(env.console, context, to_zoom_to_max, true);
        pbf_move_left_joystick(context, {-0.375, -1}, 100ms, 100ms);
        if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
            pbf_move_left_joystick(context, {-0.375, 0.75}, 100ms, 100ms);
            fly_from_map(env.console, context);
        }
    } else {
        open_map(env.console, context, to_zoom_to_max, true);
        pbf_move_left_joystick(context, {-0.5, 0}, 100ms, 100ms);
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
                pbf_move_left_joystick(context, {+1, 0}, 4000ms, 0ms);
                pbf_move_left_joystick(context, {0, -1}, 7400ms, 0ms);
                pbf_move_left_joystick(context, {-1, 0},  3000ms, 0ms);
                pbf_press_button(context, BUTTON_A, 500ms, 2500ms); // elevator up
                pbf_move_left_joystick(context, {+1, 0}, 100ms, 0ms);
                pbf_press_button(context, BUTTON_L, 100ms, 1000ms);
            },
            {black_screen}
        );
    }
    if (ret == 0){
        wait_until_overworld(env.console, context, 50s);
    }
    open_map(env.console, context, to_zoom_to_max, true);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 100ms);
    if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
        pbf_move_left_joystick(context, {+1, 0}, 100ms, 100ms);
        fly_from_map(env.console, context);
    }
    wait_until_overworld(env.console, context);
}

bool route_hyperspace_wild_zone(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_FlySpotReset_Descriptor::Stats& stats,
    bool to_zoom_to_max,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_REMAINING,
    uint8_t& ready_to_stop_counter
){
    open_map(env.console, context, false, false);
    
    // Fly from map to reset spawns
    std::shared_ptr<const ImageRGB32> overworld_screen;
    FastTravelState travel_status = fly_from_map(env.console, context, &overworld_screen);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_hyperspace_wild_zone(): Cannot fast travel after moving map cursor.",
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
            "route_hyperspace_wild_zone(): Cannot read Calorie number on screen.",
            env.console
        );
    }

    const uint16_t calorie_number = hyperspace_calorie_detector.calorie_number();
    const uint16_t min_calorie = MIN_CALORIE_REMAINING;
    const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
    env.add_overlay_log(log_msg);
    env.log(log_msg);
    // `ready_to_stop_counter` serves as a "buffer zone" to ensure one wrong calorie detection won't stop
    // the program early.
    // Here we require the code to build up this `ready_to_stop_counter` counter when calorie is approaching
    // the limit.
    if (ready_to_stop_counter < 2 && calorie_number <= min_calorie + 12){
        // We use "+12" because the 5-star Hyperspace burns about 6 Cal per sec and per reset. So we want
        // to have enough time to build up `ready_to_stop_counter` to have value 2 when we reach min_calorie.
        ready_to_stop_counter++;
    } else if (ready_to_stop_counter >= 2 && calorie_number <= min_calorie){
        // We've built up the ready_to_stop_counter so we can stop immidiately when detected calorie is lower
        // than the threshodl.
        return true;
    } else if (ready_to_stop_counter > 0 && calorie_number > min_calorie + 12){
        ready_to_stop_counter--;
    }
    return false;
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
        // Need special handling
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
    uint8_t ready_to_stop_counter = 0;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);

                bool should_stop = false;
                if (ROUTE == Route::HYPERSPACE_WILD_ZONE){
                    should_stop = route_hyperspace_wild_zone(
                        env, context, stats,
                        to_zoom_to_max,
                        MIN_CALORIE_REMAINING,
                        ready_to_stop_counter
                    );
                } else{
                    route(env, context, stats, to_zoom_to_max);
                }
                to_zoom_to_max = false;
                num_resets++;
                stats.resets++;
                env.update_stats();
                if (stats.resets.load(std::memory_order_relaxed) % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }

                if (should_stop){
                    break;
                }
                if (ROUTE == Route::HYPERSPACE_WILD_ZONE && NUM_RESETS > 0 && num_resets >= NUM_RESETS){
                    env.log(std::format("Reached reset limit {}", static_cast<uint64_t>(NUM_RESETS)));
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

    if (ROUTE == Route::HYPERSPACE_WILD_ZONE){
        go_home(env.console, context);
    }
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation

