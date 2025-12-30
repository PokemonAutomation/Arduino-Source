/*  Shiny Hunt - Shuttle Run
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShuttleRun.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_ShuttleRun_Descriptor::ShinyHunt_ShuttleRun_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-ShuttleRun", STRING_POKEMON + " LZA",
        "Shuttle Run",
        "Programs/PokemonLZA/ShinyHunt-ShuttleRun.html",
        "Shiny hunt by repeatedly running back and forth between two points",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class ShinyHunt_ShuttleRun_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Rounds"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_ShuttleRun_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_ShuttleRun::ShinyHunt_ShuttleRun()
    : DURATION("<b>Duration:</b><br>Run the program this long.", LockMode::UNLOCK_WHILE_RUNNING, "5 h")
    , ROUTE("<b>Hunt Route:</b>",
        {
            {Route::WILD_ZONE_3_TOWER, "wild_zone_3_tower", "Wild Zone 3 Tower"},
            {Route::ALPHA_PIDGEOT, "alpha_pidgeot", "Alpha Pidgeot (Jaune Sector 4)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Route::WILD_ZONE_3_TOWER
    )
    , SHINY_DETECTED("Shiny Detected", "", "1000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(DURATION);
    PA_ADD_OPTION(ROUTE);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {


void route_alpha_pidgeot(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    int ret = -1;
    {
        BlackScreenOverWatcher black_screen(COLOR_BLUE);
        ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 4000ms, 0ms);
                pbf_move_left_joystick(context, {0, -1}, 4500ms, 0ms);
                pbf_wait(context, 500ms);
            },
            {black_screen}
        );
    }
    if (ret == 0){
        wait_until_overworld(env.console, context, 50s);
        open_map(env.console, context, false);
        pbf_move_left_joystick(context, {+0.157, +0.156}, 100ms, 200ms);
        if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_map(): Unable to fast travel",
                env.console);
        }
        wait_until_overworld(env.console, context);
        ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
        pbf_move_left_joystick(context, {+1, 0}, 800ms, 0ms);
        pbf_move_left_joystick(context, {0, +1}, 4400ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0},  300ms, 0ms);
        pbf_press_button(context, BUTTON_A, 500ms, 2500ms); // elevator up
        ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
        pbf_move_left_joystick(context, {0, +1}, 900ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0},  2000ms, 0ms);
        pbf_move_left_joystick(context, {+1, 0}, 200ms, 200ms);
        pbf_press_button(context, BUTTON_L, 100ms, 200ms);
    }
}

void route_wild_zone_3_tower(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    bool been_at_downstairs = false;
    bool been_at_upstairs_after_downstairs = false;

    for(int i = 0; i < 6; i++){
        // if there is no day/night change and no button drop, this loop should only have three iterations
        const double direction = get_facing_direction(env.console, context);
        const bool face_east = get_angle_between_facing_directions(direction, 90.0) < 10.0;
        const bool face_west = get_angle_between_facing_directions(direction, 270.0) < 10.0;
        const bool face_south = get_angle_between_facing_directions(direction, 180.0) < 10.0;
        const bool face_north = get_angle_between_facing_directions(direction, 0.0) < 10.0;
        if (face_east || face_west){ // we are at downstars
            been_at_downstairs = true;
        }
        else if (been_at_downstairs){
            // we are not at downstairs right now, but we've been to the downstairs, so we 
            // must be at upstairs
            been_at_upstairs_after_downstairs = true;
        }
        if (face_east || face_south){
            // if facing east or south, run backward
            pbf_move_left_joystick(context, {0, -1}, 500ms, 200ms);
        } else if (face_west || face_north){
            // if facing west or north, run forward
            pbf_move_left_joystick(context, {0, +1}, 500ms, 200ms);
        } else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "route_wild_zone_3_tower: unexpected facing direction: " + PokemonAutomation::tostr_fixed(direction, 0) + " deg",
                env.console
            );
        }
        context.wait_for_all_requests();
        wait_until_overworld(env.console, context, 50s);

        if (been_at_upstairs_after_downstairs){
            // we've finished one run of the tower
            return;
        }
    }
}

} // namespace

void ShinyHunt_ShuttleRun::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_ShuttleRun_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_ShuttleRun_Descriptor::Stats>();
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
    std::function<void(SingleSwitchProgramEnvironment&, ProControllerContext&)> route;
    switch (ROUTE) {
    case Route::ALPHA_PIDGEOT:
        route = route_alpha_pidgeot;
        break;
    case Route:: WILD_ZONE_3_TOWER:
        route = route_wild_zone_3_tower;
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route not implemented",
            env.console
        );
    }
    WallClock start_time = current_time();
    pbf_press_button(context, BUTTON_L, 100ms, 100ms); // connect

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            do{
                const std::string log_msg = "Round " + std::to_string(stats.resets + 1);
                env.log(log_msg);
                env.console.overlay().add_log(log_msg);
                shiny_sound_handler.process_pending(context);
                send_program_status_notification(env, NOTIFICATION_STATUS);
                route(env, context);
                context.wait_for_all_requests();
                stats.resets++;
                env.update_stats();
            }while (current_time() < start_time + DURATION.get());
        },
        {{shiny_detector}}
    );

    shiny_sound_handler.process_pending(context);
    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
