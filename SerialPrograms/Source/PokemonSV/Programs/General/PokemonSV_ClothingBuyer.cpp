/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_ClothingTopDetector.h"
#include "PokemonSV_ClothingBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

ClothingBuyer_Descriptor::ClothingBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ClothingBuyer",
        STRING_POKEMON + " SV", "Clothing Buyer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ClothingBuyer.md",
        "Buy all the clothing in a store.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

ClothingBuyer::ClothingBuyer()
    : USE_LP(
        "<b>Use LP to purchase:</b>",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , WEAR_NEW_CLOTHES(
        "<b>Wear new clothing after purchase:</b>",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , NUM_CATEGORY(
        "<b>Number of Categories:</b><br>The number of categories of clothing the shop has.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 5
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(USE_LP);
    PA_ADD_OPTION(WEAR_NEW_CLOTHES);
    PA_ADD_OPTION(NUM_CATEGORY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ClothingBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    /*
    * Start at the top of the first category in clothing shop menu.
    * Program will buy everything in num of categories.
    * Can set num category higher or lower and will still work.
    * Lower - can buy only one category by setting to one and starting program on desired clothes type
    * Higher - will take longer to finish as it will repeat an already purchased category
    * Stop conditions: Out of money/lp, hit catogory count
    */

    uint8_t category_rotation_count = 0;
    bool finish_program = false;
    while (category_rotation_count < NUM_CATEGORY){
        pbf_press_button(context, BUTTON_A, 20, 100);

        AdvanceDialogWatcher already_purchased(COLOR_RED);
        PromptDialogWatcher buy_yes_no(COLOR_CYAN);
        PromptDialogWatcher hairstyle_purchase_prompt(COLOR_RED);
        int retP = 0;
        bool incompatible_hairstyle = false;

        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(10),
            { already_purchased, buy_yes_no }
        );
        switch (ret){
        case 0:
                env.log("Item already purchased or incompatible hairstyle.");
                pbf_press_button(context, BUTTON_A, 10, 100);

                retP = wait_until(
                    env.console, context,
                    std::chrono::seconds(2),
                    { hairstyle_purchase_prompt }
                );
                if (retP != 0){
                    env.log("Item already purchased.");
                    send_program_status_notification(
                        env, NOTIFICATION_STATUS_UPDATE,
                        "Item already purchased."
                    );
                    break;
                }
                env.log("Incompatible hairstyle. Proceed to purchase.");
                incompatible_hairstyle = true;
        case 1:
        {
            PromptDialogWatcher wear_yes_no(COLOR_CYAN);
            AdvanceDialogWatcher afford_yes_no(COLOR_RED);
            AdvanceDialogWatcher afford_yes_no_hairstyle(COLOR_RED);
            int retHairstyle;

            env.log("Detected purchase prompt.");

            if (USE_LP){
                env.log("Using LP.");
                pbf_press_dpad(context, DPAD_DOWN, 10, 100);
            }
            env.log("Purchasing.");
            //Purchase, then wait a bit more for wear prompt detector.
            pbf_press_button(context, BUTTON_A, 10, 100);
            pbf_wait(context, 100);
            context.wait_for_all_requests();

            int retWear = wait_until(
                env.console, context,
                std::chrono::seconds(10),
                { wear_yes_no, afford_yes_no }
            );
            switch (retWear){
            case 0:
                if (!WEAR_NEW_CLOTHES){
                    env.log("Do not wear new clothes.");
                    pbf_press_dpad(context, DPAD_DOWN, 10, 100);
                }else{
                    env.log("Wear new clothes.");
                }
                pbf_press_button(context, BUTTON_A, 10, 100);
                env.log("Clothing purchased. Selecting next item.");
                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Clothing purchased. Selecting next item."
                );
                break;
            case 1:
                if (!incompatible_hairstyle){
                    env.log("Out of Cash/LP.");
                    send_program_status_notification(
                        env, NOTIFICATION_STATUS_UPDATE,
                        "Out of Cash/LP."
                    );
                    finish_program = true;
                }else{
                    pbf_press_button(context, BUTTON_A, 10, 100);
                    retHairstyle = wait_until(
                        env.console, context,
                        std::chrono::seconds(2),
                        { afford_yes_no_hairstyle }
                    );
                    if (retHairstyle == 0){
                        env.log("Out of Cash/LP.");
                        send_program_status_notification(
                            env, NOTIFICATION_STATUS_UPDATE,
                            "Out of Cash/LP."
                        );
                        finish_program = true;
                    }
                }
                break;
            default:
                env.log("Error looking for wear prompt.");
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Error looking for wear prompt.",
                    env.console
                );
                break;
            }
            break;
        }
        default:
                env.log("Error looking for purchase prompt.");
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Error looking for purchase prompt.",
                    env.console
                );
                break;
        }
        //Out of cash/LP, stop.
        if (finish_program){
            break;
        }

        env.log("Moving on to next item.");
        pbf_press_dpad(context, DPAD_DOWN, 10, 100);
        //Wait to load a bit for next item
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        ClothingTopWatcher top_item(COLOR_YELLOW);

        int retTop = wait_until(
            env.console, context,
            std::chrono::seconds(1),
            { top_item }
        );
        if (retTop == 0){
            env.log("Reached top of category.");
            if (NUM_CATEGORY > 1){
                env.log("Category rotation set. Moving to next category.");
                pbf_press_dpad(context, DPAD_RIGHT, 10, 100);
                pbf_wait(context, 100);
                context.wait_for_all_requests();
                category_rotation_count++;
            }else{
                env.log("No category rotation. Ending program.");
                break;
            }
        }

    }
    env.log("Category num hit. Ending program.");
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

