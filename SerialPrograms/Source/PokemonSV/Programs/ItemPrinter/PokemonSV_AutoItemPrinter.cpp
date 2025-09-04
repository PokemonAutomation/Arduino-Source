/*  Auto Item Printer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV_ItemPrinterTools.h"
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
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        true
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
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NUM_ROUNDS(
        "<b>Number of Rounds to Run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void AutoItemPrinter::enter_printing_mode(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.console.log("Entering printing mode...");

    while (true){
        OverworldWatcher     overworld(env.console, COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        PromptDialogWatcher  prompt(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        context.wait_for_all_requests();

        int ret_printer_entry = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            { material, overworld, dialog, prompt }
        );
        context.wait_for_all_requests();

        switch (ret_printer_entry){
        case 0: // material
            env.console.log("Material selection screen detected.");
            return;
        case 1: // overworld
        case 2: // dialog
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3: // prompt
            env.console.log("Prompt detected.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_printing_mode(): No recognized state after 120 seconds.",
                env.console
            );
        }
    }
}




void AutoItemPrinter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutoItemPrinter_Descriptor::Stats& stats = env.current_stats<AutoItemPrinter_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 0);

    enter_printing_mode(env, context);

    for (uint16_t i = 0; i < NUM_ROUNDS; i++){
        item_printer_start_print(
            env.console, context, LANGUAGE,
            ItemPrinterJobs::Jobs_10
        );
        item_printer_finish_print(
            env.console, context, Language::None
        );

        env.console.log("Print completed.");
        stats.m_rounds++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }

    press_Bs_to_back_to_overworld(env.program_info(), env.console, context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
