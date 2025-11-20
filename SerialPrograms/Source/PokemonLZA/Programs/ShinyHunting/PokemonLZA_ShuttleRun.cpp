/*  Shiny Hunt - Shuttle Run
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
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
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_ShuttleRun_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_ShuttleRun::ShinyHunt_ShuttleRun()
    : DURATION("<b>Duration:</b><br>Run the program this long.", LockMode::LOCK_WHILE_RUNNING, "1 h")
    // , RUN_FORWARD_DURATION("<b>Run Forward Duration</b><br>"
    //     "Run forward and backward for this long each round trip.", LockMode::LOCK_WHILE_RUNNING, "5000 ms")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(DURATION);
    // PA_ADD_OPTION(RUN_FORWARD_DURATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

void hunt_loop(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Milliseconds duration){
    context.wait_for_all_requests();
    ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, duration, 0ms);
    pbf_move_left_joystick(context, 255, 128, 1300ms, 0ms);
    pbf_press_button(context, BUTTON_L, 100ms, 500ms);
    pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms); // open map
    context.wait_for_all_requests();
    pbf_wait(context, 500ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 100ms); // select fly point
    pbf_mash_button(context, BUTTON_A, 1000ms);
    context.wait_for(std::chrono::milliseconds(4000));
}
} // namespace

void ShinyHunt_ShuttleRun::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_ShuttleRun_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_ShuttleRun_Descriptor::Stats>();
    WallClock deadline = current_time() + DURATION.get();
    Milliseconds duration = 8500ms;
    pbf_press_button(context, BUTTON_L, 100ms, 100ms); // connect
    do{
        send_program_status_notification(env, NOTIFICATION_STATUS);
        stats.resets++;
        hunt_loop(env, context, duration);
        env.update_stats();
    }while (current_time() < deadline);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
