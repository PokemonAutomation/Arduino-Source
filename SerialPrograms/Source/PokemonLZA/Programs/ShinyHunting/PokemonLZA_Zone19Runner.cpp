/*  Shiny Hunt - Zone 19 Runner
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
#include "PokemonLZA_Zone19Runner.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_Zone19Runner_Descriptor::ShinyHunt_Zone19Runner_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-Zone19Runner", STRING_POKEMON + " LZA",
        "Wild Zone 19 Runner",
        "Programs/PokemonLZA/ShinyHunt-Zone19Runner.html",
        "Shiny hunt by repeatedly running outside of Wild Zone 19",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_Zone19Runner_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> ShinyHunt_Zone19Runner_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_Zone19Runner::ShinyHunt_Zone19Runner()
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

void fly_back_to_zone_entrance(ConsoleHandle& console, ProControllerContext& context){
    open_map(console, context, true);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 0, 128, 100ms, 100ms); // TODO: inference
    fly_from_map(console, context);
}

void fly_back_to_zone_entrance_after_day_night_change(ConsoleHandle& console, ProControllerContext& context){
    open_map(console, context, true);
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 0, 128, 100ms, 100ms); // TODO: inference
    if (fly_from_map(console, context) == FastTravelState::NOT_AT_FLY_SPOT) {
        pbf_move_left_joystick(context, 128, 255, 100ms, 100ms); // TODO: inference
        fly_from_map(console, context);
    }
}

void hunt_zone_loop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ShinyHunt_Zone19Runner_Descriptor::Stats& stats = env.current_stats<ShinyHunt_Zone19Runner_Descriptor::Stats>();
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
            pbf_move_left_joystick(context, 0, 80, 6500ms, 0ms);
        },
        {{black_screen}}
    );
    if (ret == 0){
        env.console.log("[Zone19Runner] Detected day/night change");
        stats.day_changes++;
        context.wait_for(std::chrono::milliseconds(2000));
        fly_back_to_zone_entrance_after_day_night_change(env.console, context);
    } else {
        fly_back_to_zone_entrance(env.console, context);
    }
    context.wait_for(std::chrono::milliseconds(1000));
}
} // namespace

void ShinyHunt_Zone19Runner::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_Zone19Runner_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_Zone19Runner_Descriptor::Stats>();
    WallClock deadline = current_time() + DURATION.get();
    pbf_press_button(context, BUTTON_L, 100ms, 100ms); // connect
    do{
        send_program_status_notification(env, NOTIFICATION_STATUS);
        stats.resets++;
        hunt_zone_loop(env, context);
        env.update_stats();
    }while (current_time() < deadline);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
