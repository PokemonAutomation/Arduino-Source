/*  Shiny Hunt - Zone 11 Alpha
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_Zone11Alpha.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_Zone11Alpha_Descriptor::ShinyHunt_Zone11Alpha_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-Zone11Alpha", STRING_POKEMON + " LZA",
        "Wild Zone 11 Alphas",
        "Programs/PokemonLZA/ShinyHunt-Zone11Alpha.html",
        "Shiny hunt by repeatedly running from Zone 6 to Zone 11 entrance.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_Zone11Alpha_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Rounds"])
        , day_changes(m_stats["Day/Night Changes"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Day/Night Changes");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& day_changes;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_Zone11Alpha_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_Zone11Alpha::ShinyHunt_Zone11Alpha()
    : DURATION("<b>duration:</b><br>Run the program this long.", LockMode::LOCK_WHILE_RUNNING, "1 h")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(DURATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

#if 0
void run_from_zone6_to_zone11_entrance(ConsoleHandle& console, ProControllerContext& context){
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &console.overlay());
    run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 255, 50ms, 50ms);
            pbf_move_left_joystick(context, 128, 255, 50ms, 50ms);
            pbf_press_button(context, BUTTON_L, 100ms, 100ms);
            ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
            pbf_move_left_joystick(context, 128, 0, 3700ms, 0ms);
            pbf_move_left_joystick(context, 255, 128, 200ms, 200ms);
            pbf_press_button(context, BUTTON_L, 100ms, 100ms);
            ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
            pbf_move_left_joystick(context, 128, 0, 11000ms, 0ms);
        },
        {{buttonA}}
    );
}
#else

void run_from_zone6_to_zone11_entrance(ConsoleHandle& console, ProControllerContext& context){
    {
        ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &console.overlay());
        run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                for (int c = 0; c < 10; c++){
                    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                    pbf_move_left_joystick(context, 128, 0, 800ms, 200ms);
                }
            },
            {{buttonA}}
        );
    }
    context.wait_for_all_requests();
    pbf_mash_button(context, BUTTON_A, 2000ms); // enter zone 6
    pbf_move_left_joystick(context, 0, 128, 50ms, 100ms);
    pbf_press_button(context, BUTTON_L, 50ms, 100ms);
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 5800ms, 0ms);
    pbf_move_left_joystick(context, 0, 128, 100ms, 100ms);
    pbf_press_button(context, BUTTON_L, 100ms, 100ms);
    context.wait_for_all_requests();
    {
        ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &console.overlay());
        run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                for (int c = 0; c < 10; c++){
                    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
                    pbf_move_left_joystick(context, 128, 0, 3500ms, 200ms);
                }
            },
            {{buttonA}}
        );
    }
    pbf_mash_button(context, BUTTON_A, 2000ms); // exit zone 6
    pbf_move_left_joystick(context, 128, 0, 200ms, 200ms);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 255, 88, 100ms, 200ms);
    pbf_press_button(context, BUTTON_L, 100ms, 100ms);
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 120, 0, 5000ms, 200ms);
}
#endif

void fly_to_zone6_entrance(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 192, 64, 250ms, 200ms); // TODO: inference
    fly_from_map(console, context);
}

void fly_to_zone6_entrance_after_day_night_change(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    if (fly_from_map(console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
        fly_to_zone6_entrance(console, context);
    }
}

void hunt_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ShinyHunt_Zone11Alpha_Descriptor::Stats& stats = env.current_stats<ShinyHunt_Zone11Alpha_Descriptor::Stats>();
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            run_from_zone6_to_zone11_entrance(env.console, context);
        },
        {{black_screen}}
    );
    if (ret == 0){
        env.console.log("[Zone11Alpha] Detected day/night change");
        stats.day_changes++;
        context.wait_for(std::chrono::milliseconds(2000));
        open_map(env.console, context, true);
        fly_to_zone6_entrance_after_day_night_change(env.console, context);
    } else {
        open_map(env.console, context, true);
        fly_to_zone6_entrance(env.console, context);
    }
    context.wait_for(std::chrono::milliseconds(1000));
}
} // namespace

void ShinyHunt_Zone11Alpha::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_Zone11Alpha_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_Zone11Alpha_Descriptor::Stats>();
    WallClock deadline = current_time() + DURATION.get();
    do{
        send_program_status_notification(env, NOTIFICATION_STATUS);
        stats.resets++;
        hunt_loop(env, context);
        env.update_stats();
    }while (current_time() < deadline);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
