/*  Shiny Hunt - Sewer Whirlipede Runner
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
#include "PokemonLZA_SewerWhirlipedeRunner.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_SewerWhirlipedeRunner_Descriptor::ShinyHunt_SewerWhirlipedeRunner_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-SewerWhirlipedeRunner", STRING_POKEMON + " LZA",
        "Shiny Hunt - Sewer Whirlipede Runner",
        "Programs/PokemonLZA/ShinyHunt-SewerWhirlipedeRunner.html",
        "Shiny hunt by repeatedly respawning static alpha Whirlipede, running just outside of the Lumiose Sewers.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Whirlipede Respawning"])
        , day_changes(m_stats["Day/Night Changes"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Whirlipede Respawning");
        m_display_order.emplace_back("Day/Night Changes");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& day_changes;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_SewerWhirlipedeRunner_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_SewerWhirlipedeRunner::ShinyHunt_SewerWhirlipedeRunner()
    : DURATION("<b>duration:</b><br>Run the program this long.", LockMode::UNLOCK_WHILE_RUNNING, "1 h")
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


void run_one_way(ConsoleHandle& console, ProControllerContext& context){
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &console.overlay());
    run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 255, 100ms, 100ms);
            pbf_press_button(context, BUTTON_L, 100ms, 100ms);
            ssf_press_button(context, BUTTON_B, 0ms, 500ms, 0ms);
            pbf_move_left_joystick(context, 165, 0, 3000ms, 0ms);
        },
        {{buttonA}}
    );
}

void fly_to_lumiose_sewers(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 192, 192, 50ms, 200ms); // TODO: inference
    fly_from_map(console, context);
    context.wait_for(std::chrono::milliseconds(1000));
}

void run_at_lumiose_sewers_entrance(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats& stats = env.current_stats<ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats>();
    context.wait_for_all_requests();

    BlackScreenOverWatcher black_screen(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            run_one_way(env.console, context);
        },
        {{black_screen}}
    );
    if (ret == 0){
        env.console.log("[SewerWhirlipedeRunner] Detected day/night change");
        stats.day_changes++;
        context.wait_for(std::chrono::milliseconds(2000));
    }
    open_map(env.console, context);
    fly_to_lumiose_sewers(env.console, context);
}

void ShinyHunt_SewerWhirlipedeRunner::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats>();
    WallClock deadline = current_time() + DURATION.get();
    do{
        send_program_status_notification(env, NOTIFICATION_STATUS);
        stats.resets++;
        run_at_lumiose_sewers_entrance(env, context);
        env.update_stats();
    }while (current_time() < deadline);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
