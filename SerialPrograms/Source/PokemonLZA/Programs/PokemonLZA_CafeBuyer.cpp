/*  Cafe Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA_CafeBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

CafeBuyer_Descriptor::CafeBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:CafeBuyer",
        STRING_POKEMON + " LZA", "Cafe Buyer",
        "Programs/PokemonLZA/CafeBuyer.html",
        "Repeatedly use Cafes to obtain the Cafe Regular title and raise friendship.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class CafeBuyer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : purchases(m_stats["Purchases"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Purchases");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& purchases;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> CafeBuyer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

CafeBuyer::CafeBuyer()
    : PURCHASES(
        "<b>Purchases:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        50
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(PURCHASES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void CafeBuyer::purchase_water(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool first){
    CafeBuyer_Descriptor::Stats& stats = env.current_stats<CafeBuyer_Descriptor::Stats>();
    bool exit = false;
    while (!exit){
        context.wait_for_all_requests();

        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.631, 0.382, 0.034, 0.069}
        );
        SelectionArrowWatcher selection_arrow_watcher_1(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.716, 0.456, 0.034, 0.069}
        );
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay());
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
        int ret2 = 0;
        
        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                selection_arrow_watcher_1,
                buttonA_watcher,
                selection_arrow_watcher,
                white_dialog_watcher,
                black_screen,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected 2nd selection arrow.");
            pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            exit = true;
            break;
        case 1:
            env.log("Detected A button.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 2:
            env.log("Detected 1st selection arrow.");
            if (first == true){
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
                pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            } else {
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
                pbf_press_dpad(context, DPAD_DOWN, 40ms, 40ms);
                pbf_press_button(context, BUTTON_A, 80ms, 40ms);
                exit = true;
            }
            continue;
        case 3:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 4:
            env.log("Detected black screen. Wait out the day change.");
            ret2 = wait_until(
                env.console, context,
                30s,
                {overworld}
            );
            if (ret2 < 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "purchase_water(): Unable to go back to overworld after day/night change on bench after 30 seconds.",
                    env.console
                );
            }
            continue;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "purchase_water(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void CafeBuyer::handle_purchase(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CafeBuyer_Descriptor::Stats& stats = env.current_stats<CafeBuyer_Descriptor::Stats>();
    context.wait_for_all_requests();

    env.log("Handling water purchase.");
    ButtonWatcher buttonB_watcher(
        COLOR_WHITE,
        ButtonType::ButtonB,
        {0.777, 0.880, 0.222, 0.119},
        &env.console.overlay()
    );

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 10000ms);
        },
        {buttonB_watcher}
    );
    switch (ret){
    case 0:
        env.log("Sitting at table.");
        pbf_press_button(context, BUTTON_B, 80ms, 40ms);
        break;
    default:
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "handle_purchase(): Did not detect B button. Did you run out of money?",
            env.console
        );
    }
    context.wait_for_all_requests();

    bool exit = false;
    while (!exit){
        context.wait_for_all_requests();

        BlueDialogWatcher blue_dialog_watcher(COLOR_BLUE, &env.console.overlay());
        SelectionArrowWatcher selection_arrow_watcher(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.716, 0.601, 0.034, 0.066}
        );
        BlackScreenOverWatcher black_screen(COLOR_BLUE);
        OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());

        int ret2 = 0;
        int ret3 = wait_until(
            env.console, context,
            10000ms,
            {
                blue_dialog_watcher,
                selection_arrow_watcher,
                black_screen,
            }
        );
        context.wait_for(100ms);

        switch (ret3){
        case 0:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            break;
        case 1:
            env.log("Detected selection arrow.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 2:
            env.log("Detected black screen.");
            ret2 = wait_until(
                env.console, context,
                30s,
                {overworld}
            );
            if (ret2 < 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "handle_purchase(): Unable to go back to overworld after drinking water.",
                    env.console
                );
            }
            exit = true;
            break;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "handle_purchase(): No recognized state after 10 seconds.",
                env.console
            );
        }
    }
}

void CafeBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CafeBuyer_Descriptor::Stats& stats = env.current_stats<CafeBuyer_Descriptor::Stats>();

    /*
    * Cheapest: Fresh Water/Sparkling Water at any of the Nouveau Cafe
    * Truck No. 1 has extra option for same as last time, other trucks do not.
    * 
    * Req. truck 1 since repeat order is faster after the initial purchase.
    * 
    * If out of money they do not give it for free. Does not check for out of money.
    */

    //Initial purchase to set Fresh Water as "same as last time"
    bool round1 = true;
    for (uint32_t i = 0; i < PURCHASES; i++){
        purchase_water(env, context, round1);
        handle_purchase(env, context);
        stats.purchases++;
        env.update_stats();
        env.log("Purchased water.");
        send_program_status_notification(env, NOTIFICATION_STATUS);
        round1 = false;
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

