/*  Auto Item Printer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV_AutoItemPrinter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


AutoItemPrinter_Descriptor::AutoItemPrinter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:AutoItemPrinter",
        STRING_POKEMON + " SV", "Auto Item Printer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/AutoItemPrinter.md",
        "Automate the Item Printer for rare items.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct AutoItemPrinter_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_rounds(m_stats["Rounds"])
    {
        m_display_order.emplace_back("Rounds");
    }
    std::atomic<uint64_t>& m_rounds;
};
std::unique_ptr<StatsTracker> AutoItemPrinter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



AutoItemPrinter::AutoItemPrinter()
    : GO_HOME_WHEN_DONE(false)
    , NUM_ROUNDS(
        "<b>Number of Rounds to Run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void AutoItemPrinter::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutoItemPrinter_Descriptor::Stats& stats = env.current_stats<AutoItemPrinter_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 0);

    stats.m_trials++;
    env.update_stats();

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
