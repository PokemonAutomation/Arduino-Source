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
            {Routes::KLEFKI,  "klefki",  "Sewers: Klefki"},
            {Routes::KLEFKI_INKAY_GOOMY,  "klefki_inkay_goomy",  "Sewers: Klefki+Inkay+Goomy"},
            {Routes::LITWICK,  "litwick",  "Sewers: Litwick"},
            {Routes::SKRELP,  "skrelp",  "Sewers: Skrelp"},
            {Routes::SKRELP_INKAY,  "skrelp_inkay",  "Sewers: Skrelp+Inkay"},
            {Routes::SKRELP_ARIADOS,  "skrelp_ariados",  "Sewers: Skrelp+Ariados"},
            // {Routes::SCRAGGY,  "scraggy",  "Sewers: Scraggy"},
            {Routes::WILD_ZONE_19, "wild_zone_19", "Wild Zone 19"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Routes::KLEFKI
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

void fly_back_to_sewers_entrance(ConsoleHandle& console, ProControllerContext& context) {
    pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms); // open map
    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 100ms); // select fly point
    {
        BlackScreenOverWatcher black_screen(COLOR_BLUE);
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 10000ms);
            },
            {black_screen}
        );
        if (ret != 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_back_to_sewers_entrance(): cannot detect black screen after mashing A.",
                console
            );
        }
    }
    wait_until_overworld(console, context);
}

void route_klefki(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 4900ms, 0ms);
    pbf_move_left_joystick(context, 0, 128, 1000ms, 0ms);
    pbf_press_button(context, BUTTON_L, 100ms, 500ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_klefki_inkay_goomy(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 8500ms, 0ms);
    pbf_move_left_joystick(context, 255, 128, 1300ms, 0ms);
    pbf_press_button(context, BUTTON_L, 100ms, 500ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_litwick(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 5000ms, 0ms);
    pbf_move_left_joystick(context, 128, 255, 5500ms, 0ms);
    pbf_wait(context, 500ms);
}

void route_skrelp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    fly_back_to_sewers_entrance(env.console, context);
    pbf_wait(context, 1000ms);
}

void route_skrelp_inkay(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 3900ms, 0ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_skrelp_ariados(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 6000ms, 0ms);
    pbf_move_left_joystick(context, 128, 255, 6500ms, 0ms);
    pbf_wait(context, 500ms);
}

void route_scraggy(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
//TODO
}

void route_wild_zone_19(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
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
    case Routes::KLEFKI:
        route = route_klefki;
        break;
    case Routes::KLEFKI_INKAY_GOOMY:
        route = route_klefki_inkay_goomy;
        break;
    case Routes::LITWICK:
        route = route_litwick;
        break;
    case Routes::SKRELP:
        route = route_skrelp;
        break;
    case Routes::SKRELP_INKAY:
        route = route_skrelp_inkay;
        break;
    case Routes::SKRELP_ARIADOS:
        route = route_skrelp_ariados;
        break;
    case Routes::SCRAGGY:
        route = route_scraggy;
        break;
    case Routes::WILD_ZONE_19:
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
                stats.resets++;
                route(env, context);
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
