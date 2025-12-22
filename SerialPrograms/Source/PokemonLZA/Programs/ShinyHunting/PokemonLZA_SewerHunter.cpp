/*  Shiny Hunt - Sewer Hunter
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
#include "PokemonLZA_SewerHunter.h"

namespace PokemonAutomation::NintendoSwitch::PokemonLZA {

using namespace Pokemon;


ShinyHunt_SewerHunter_Descriptor::ShinyHunt_SewerHunter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-SewerHunter", STRING_POKEMON + " LZA",
        "Sewer Hunter",
        "Programs/PokemonLZA/ShinyHunt-SewerHunter.html",
        "Shiny hunt in two sewer subzones",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_SewerHunter_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> ShinyHunt_SewerHunter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_SewerHunter::ShinyHunt_SewerHunter()
    : DURATION("<b>Duration:</b><br>Run the program this long.", LockMode::UNLOCK_WHILE_RUNNING, "5 h")
    , ROUTE("<b>Hunt Route:</b>",
        {
            {Route::KLEFKI,  "klefki",  "Klefki"},
            {Route::KLEFKI_INKAY_GOOMY,  "klefki_inkay_goomy",  "Klefki+Inkay+Goomy"},
            {Route::LITWICK,  "litwick",  "Litwick"},
            {Route::LITWICK_SKRELP,  "litwick_skrelp",  "Litwick+Skrelp+Haunter"},
            {Route::SKRELP,  "skrelp",  "Skrelp"},
            {Route::SKRELP_INKAY,  "skrelp_inkay",  "Skrelp+Inkay"},
            {Route::ARIADOS,  "ariados",  "Ariados"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Route::KLEFKI
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

void run_forward_backward_to_wall(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    PokemonAutomation::Milliseconds duration
){
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, duration, 0ms);
    pbf_move_left_joystick(context, 128, 255, duration + 500ms, 0ms);
    pbf_wait(context, 500ms);
}

void route_klefki(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 4900ms, 0ms);
    pbf_move_left_joystick(context, {-1, 0},  1000ms, 0ms);
    pbf_press_button(context, BUTTON_L, 100ms, 500ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_klefki_inkay_goomy(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 8500ms, 0ms);
    pbf_move_left_joystick(context, 255, 128, 1300ms, 0ms);
    pbf_press_button(context, BUTTON_L, 100ms, 500ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_litwick(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    run_forward_backward_to_wall(env, context, 5s);
}

void route_litwick_skrelp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    run_forward_backward_to_wall(env, context, 9000ms);
}

void route_skrelp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    fly_back_to_sewers_entrance(env.console, context);
    pbf_wait(context, 1000ms);
}

void route_skrelp_inkay(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 3900ms, 0ms);
    fly_back_to_sewers_entrance(env.console, context);
}

void route_ariados(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    run_forward_backward_to_wall(env, context, 6s);
}

} // namespace


void ShinyHunt_SewerHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_SewerHunter_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_SewerHunter_Descriptor::Stats>();
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
    case Route::KLEFKI:
        route = route_klefki;
        break;
    case Route::KLEFKI_INKAY_GOOMY:
        route = route_klefki_inkay_goomy;
        break;
    case Route::LITWICK:
        route = route_litwick;
        break;
    case Route::LITWICK_SKRELP:
        route = route_litwick_skrelp;
        break;
    case Route::SKRELP:
        route = route_skrelp;
        break;
    case Route::SKRELP_INKAY:
        route = route_skrelp_inkay;
        break;
    case Route::ARIADOS:
        route = route_ariados;
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


}  // namespace PokemonAutomation::NintendoSwitch::PokemonLZA
