/*  Auto Item Printer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
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
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void AutoItemPrinter::enter_printing_mode(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Entering printing mode...");

    while (true){
        OverworldWatcher     overworld(COLOR_CYAN);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        PromptDialogWatcher  prompt(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.70, 0.93, 0.10, 0.06});
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
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "enter_printing_mode(): No recognized state after 120 seconds.",
                true
            );
        }
    }
}


void AutoItemPrinter::start_print(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Starting print...");

    while (true){
        PromptDialogWatcher  prompt(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.70, 0.93, 0.10, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        context.wait_for_all_requests();

        int ret_print_start = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            { handle, prompt, material }
        );
        context.wait_for_all_requests();

        switch (ret_print_start){
        case 0: // handle
            return;
        case 1: // prompt
            env.console.log("Confirming material selection...");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2: // material
            pbf_press_button(context, BUTTON_X, 20, 230);
            continue;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "start_print(): No recognized state after 120 seconds.",
                true
            );
        }
    }
}


void AutoItemPrinter::finish_print(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Finishing print...");

    while (true){
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        WhiteButtonWatcher   material(COLOR_GREEN, WhiteButton::ButtonX, {0.70, 0.93, 0.10, 0.06});
        WhiteButtonWatcher   handle(COLOR_BLUE,    WhiteButton::ButtonA, {0.40, 0.80, 0.20, 0.14});
        WhiteButtonWatcher   result(COLOR_CYAN,    WhiteButton::ButtonA, {0.87, 0.93, 0.10, 0.06});
        context.wait_for_all_requests();

        int ret_print_end = wait_until(
            env.console, context,
            std::chrono::seconds(120),
            { material, handle, dialog, result }
        );
        context.wait_for_all_requests();

        switch (ret_print_end){
        case 0: // material
            return;
        case 1: // handle
        case 2: // dialog
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3: // result
            env.console.log("Result screen detected.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "finish_print(): No recognized state after 120 seconds.",
                true
            );
        }
    }
}


void AutoItemPrinter::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    AutoItemPrinter_Descriptor::Stats& stats = env.current_stats<AutoItemPrinter_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 0);

    enter_printing_mode(env, context);

    for (uint16_t i = 0; i < NUM_ROUNDS; i++){
        start_print(env, context);
        finish_print(env, context);

        env.console.log("Print completed.");
        stats.m_rounds++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }

    env.console.log("Returning to overworld...");
    OverworldWatcher overworld(COLOR_CYAN);
    context.wait_for_all_requests();

    int ret_finish = run_until(
        env.console, context,
        [](BotBaseContext& context) {
            pbf_press_button(context, BUTTON_B, 20, 105);
        },
        { overworld }
    );
    context.wait_for_all_requests();

    if (ret_finish < 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Couldn't return to overworld after 80 seconds.",
            true
        );
    }
    
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
