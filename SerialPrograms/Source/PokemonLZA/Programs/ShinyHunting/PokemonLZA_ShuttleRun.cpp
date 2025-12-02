/*  Shiny Hunt - Shuttle Run
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
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
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
            // {Route::SCRAGGY,  "scraggy",  "Sewers: Scraggy"},
            {Route::WILD_ZONE_19, "wild_zone_19", "Wild Zone 19"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Route::WILD_ZONE_19
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


void route_scraggy(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
//TODO
}

void route_wild_zone_19(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (run_a_straight_path_in_overworld(env.console, context, 0, 80, 6500ms) == 0) {
        open_map(env.console, context, false);
        pbf_move_left_joystick(context, 0, 128, 100ms, 100ms);
        if (fly_from_map(env.console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
            pbf_move_left_joystick(context, 128, 255, 100ms, 100ms);
            fly_from_map(env.console, context);
        }
    } else {
        open_map(env.console, context, false);
        pbf_move_left_joystick(context, 0, 128, 100ms, 100ms);
        fly_from_map(env.console, context);
    }
    wait_until_overworld(env.console, context, 50s);
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
    case Route::SCRAGGY:
        route = route_scraggy;
        break;
    case Route::WILD_ZONE_19:
        route = route_wild_zone_19;
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
