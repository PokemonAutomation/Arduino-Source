/*  Shiny Hunt - Overworld Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_ShinyHunt_OverworldReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_OverworldReset_Descriptor::ShinyHunt_OverworldReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-OverworldReset",
        STRING_POKEMON + " LZA", "Shiny Hunt - Overworld Reset",
        "Programs/PokemonLZA/ShinyHunt-OverworldReset.html",
        "Shiny hunt by repeatedly sitting on a bench to reset spawns.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class ShinyHunt_OverworldReset_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_OverworldReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_OverworldReset::ShinyHunt_OverworldReset()
    : RESET_DELAY(
        "<b>Reset Delay:</b><br>Wait this long after the game loads before resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , ROTATE_CAMERA(
        "<b>Rotate Camera:</b><br>Rotate camera upon entering the game.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SHINY_DETECTED(
        "Shiny Detected", "",
        "5000 ms",
        ShinySoundDetectedAction::STOP_PROGRAM
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
    PA_ADD_OPTION(RESET_DELAY);
    PA_ADD_OPTION(ROTATE_CAMERA);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHunt_OverworldReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_OverworldReset_Descriptor::Stats& stats = env.current_stats<ShinyHunt_OverworldReset_Descriptor::Stats>();

    uint8_t shiny_count = 0;

    while (true){
        go_home(env.console, context);
        send_program_status_notification(env, NOTIFICATION_STATUS);


        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            shiny_count++;
            stats.shinies++;
            env.update_stats();
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                reset_game_from_home(env, env.console, context);

                if (ROTATE_CAMERA){
                    pbf_move_right_joystick(context, 255, 128, RESET_DELAY, 0ms);
                }else{
                    pbf_wait(context, RESET_DELAY);
                }

                stats.resets++;
                env.update_stats();
            },
            {{shiny_detector}}
        );

        if (ret < 0){
            continue;
        }

        if (SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            shiny_count,
            shiny_coefficient
        )){
            break;
        }
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}










}
}
}
