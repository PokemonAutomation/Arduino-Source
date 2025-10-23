/*  Shiny Hunt - Bench
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include <sstream>
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_Bench.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_Bench_Descriptor::ShinyHunt_Bench_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-Bench",
        STRING_POKEMON + " LZA", "Shiny Hunt - Bench",
        "Programs/PokemonLZA/ShinyHunt-Bench.html",
        "Shiny hunt by repeatedly sitting on a bench to reset spawns.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class ShinyHunt_Bench_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Bench Sits"])
        , shinies(m_stats["Shinies Detected"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Bench Sits");
        m_display_order.emplace_back("Shinies Detected");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Shinies"] = "Shinies Detected";
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_Bench_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_Bench::ShinyHunt_Bench()
    : SHINY_DETECTED("Shiny Detected", "", "2000ms")
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

void ShinyHunt_Bench::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_Bench_Descriptor::Stats& stats = env.current_stats<ShinyHunt_Bench_Descriptor::Stats>();

    uint8_t shiny_count = 0;

    while (true){
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
                while (true){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                    sit_on_bench(env.console, context);
                    stats.resets++;
                    env.update_stats();
                }
            },
            {{shiny_detector}}
        );

        //  This should never happen.
        if (ret != 0){
            continue;
        }

        shiny_count++;
        if (on_shiny_sound(
            env, env.console, context,
            SHINY_DETECTED,
            shiny_count,
            shiny_coefficient
        )){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}














}
}
}
