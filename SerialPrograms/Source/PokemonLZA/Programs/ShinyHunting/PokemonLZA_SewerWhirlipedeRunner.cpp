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
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
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
        "Shiny hunt by repeatedly respawning statis alpha Whirlipede, running just outside of the Lumiose Sewers.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Whirlipede Respawning"])
        , day_changes(m_stats["Day/Night Changes"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Whirlipede Respawning");
        m_display_order.emplace_back("Day/Night Changes");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& day_changes;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_SewerWhirlipedeRunner_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_SewerWhirlipedeRunner::ShinyHunt_SewerWhirlipedeRunner()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
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
    PA_ADD_OPTION(SHINY_DETECTED);
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

bool fly_to_lumiose_sewers(ConsoleHandle& console, ProControllerContext& context){
    context.wait_for_all_requests();
    pbf_move_left_joystick(context, 192, 192, 50ms, 100ms); // TODO: inference
    return fly_from_map(console, context);
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
        env.console.log("[SewerWhirlipedeRunner] Detected day/night change after entering.");
        stats.day_changes++;
        context.wait_for(std::chrono::milliseconds(2000)); // TODO: wait for ending
    }
    open_map(env.console, context);
    fly_to_lumiose_sewers(env.console, context);
}

void ShinyHunt_SewerWhirlipedeRunner::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_SewerWhirlipedeRunner_Descriptor::Stats>();

    if (SHINY_DETECTED.ACTION == ShinySoundDetectedAction::NOTIFY_ON_ALL){
        throw UserSetupError(
            env.console,
            "Shiny would be detected/notified at most once. Choose one of the other 2 options."
        );
    }

    while (true){
        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            env.update_stats();
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                while (true){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                    stats.resets++;
                    run_at_lumiose_sewers_entrance(env, context);
                    env.update_stats();
                }
            },
            {{shiny_detector}}
        );

        //  This should never happen.
        if (ret != 0){
            continue;
        }

        bool exit = SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            stats.shinies,
            shiny_coefficient
        );

        open_map(env.console, context);
        if (!fly_to_lumiose_sewers(env.console, context)){
            pbf_mash_button(context, BUTTON_B, 5000ms);
        }

        if (exit){
            break;
        }
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
