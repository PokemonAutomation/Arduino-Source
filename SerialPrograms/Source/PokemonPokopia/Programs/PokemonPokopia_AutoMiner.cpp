/*  Auto Miner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"
#include "PokemonPokopia/Inference/PokemonPokopia_MovesDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_PCDetection.h"
#include "PokemonPokopia/Inference/PokemonPokopia_SettingsScreenDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

#include "PokemonPokopia/Programs/PokemonPokopia_PCNavigation.h"
#include "PokemonPokopia/Programs/PokemonPokopia_AutoMiner.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

using namespace Pokemon;


AutoMiner_Descriptor::AutoMiner_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonPokopia:AutoMiner",
        STRING_POKEMON + " Pokopia", "Auto Miner",
        "Programs/PokemonPokopia/AutoMiner.html",
        "Automatically mine resources on a Dream Island.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class AutoMiner_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> AutoMiner_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AutoMiner::AutoMiner()
    : GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void AutoMiner::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // AutoMiner_Descriptor::Stats& stats = env.current_stats<AutoMiner_Descriptor::Stats>();

    PPWatcher pp_watcher(COLOR_YELLOW, &env.console.overlay());
    wait_until(
        env.console, context,
        60s,
        {pp_watcher}
    );
    env.console.log("percent: " + std::to_string(pp_watcher.get_pp_percent() * 100) + "%, powered up: " + (pp_watcher.is_powered_up() ? "yes" : "no"));

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
