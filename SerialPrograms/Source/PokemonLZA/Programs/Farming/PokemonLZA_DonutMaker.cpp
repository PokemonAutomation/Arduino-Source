/*  Donut Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_DonutMaker.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


DonutMaker_Descriptor::DonutMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:DonutMaker", STRING_POKEMON + " LZA",
        "Donut Maker",
        "Programs/PokemonLZA/DonutMaker.html",
        "TODO: Add description",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class DonutMaker_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        // TODO: Add more stats here
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> DonutMaker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DonutMaker::DonutMaker()
    : NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    // TODO: Add options here using PA_ADD_OPTION()
    PA_ADD_OPTION(NOTIFICATIONS);
}


// Return true if it should stop
// Start the iteration at closest pokemon center
bool donut_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    return false;
}


void DonutMaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    assert_16_9_1080p_min(env.logger(), env.console);

    while(true){
        const bool should_stop = donut_iteration(env, context);
        stats.resets++;
        env.update_stats();

        if (should_stop){
            break;
        }
    }
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
