/*  Shiny Hunt - Bench Sit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_BenchSit.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_BenchSit_Descriptor::ShinyHunt_BenchSit_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-BenchSit",
        STRING_POKEMON + " LZA", "Shiny Hunt - Bench Sit",
        "Programs/PokemonLZA/ShinyHunt-BenchSit.html",
        "Shiny hunt by repeatedly sitting on a bench to reset spawns.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class ShinyHunt_BenchSit_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Bench Sits"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Bench Sits");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Shinies"] = "Shiny Sounds";
        m_aliases["Shinies Detected"] = "Shiny Sounds";
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_BenchSit_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_BenchSit::ShinyHunt_BenchSit()
    : WALK_FORWARD_DURATION(
        "<b>Walk Forward Duration</b><br>"
        "Walk forward and backward for this long after each day change to "
        "increase the spawn radius. Set to zero to disable this.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , SHINY_DETECTED(
        "Shiny Detected", "",
        "2000 ms",
        ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY
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
    PA_ADD_OPTION(WALK_FORWARD_DURATION);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHunt_BenchSit::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_BenchSit_Descriptor::Stats& stats = env.current_stats<ShinyHunt_BenchSit_Descriptor::Stats>();

    while (true){
        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
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
                    sit_on_bench(env.console, context);
                    Milliseconds duration = WALK_FORWARD_DURATION;
                    if (duration > Milliseconds::zero()){
                        ssf_press_button(context, BUTTON_B, 0ms, 2 * duration, 0ms);
                        pbf_move_left_joystick(context, 128, 0, duration, 0ms);
                        pbf_move_left_joystick(context, 128, 255, duration + 500ms, 0ms);
                    }
                    env.update_stats();
                }
            },
            {{shiny_detector}}
        );

        //  This should never happen.
        if (ret != 0){
            continue;
        }

        pbf_mash_button(context, BUTTON_B, 1000ms);

        bool exit = SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            stats.shinies,
            shiny_coefficient
        );

        pbf_move_left_joystick(context, 128, 255, WALK_FORWARD_DURATION, 0ms);

        if (exit){
            break;
        }
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}














}
}
}
