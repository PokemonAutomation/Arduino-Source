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
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA_ClothingBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

ClothingBuyer_Descriptor::ClothingBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ClothingBuyer",
        STRING_POKEMON + " LZA", "Clothing Buyer",
        "Programs/PokemonLZA/ClothingBuyer.html",
        "Buy all the clothing in a store.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

ClothingBuyer::ClothingBuyer()
    : NUM_CATEGORY(
        "<b>Number of Categories:</b><br>The number of categories of clothing the shop has.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 9
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(NUM_CATEGORY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ClothingBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    /*
    * Similar to SV clothing buyer.
    * Start at the top of the first category in clothing shop menu.
    * Buy everything in num of categories, can set num category, will wrap around.
    * Stop conditions: hit category count
    * Can't detect out of money, same dialog box as already bought.
    * Hairstyles all seem to be hat compatible in this game. Tried w/pigtails, which wasn't compatible in SV.
    */

    uint8_t category_rotation_count = 0;
    //bool finish_program = false;
    while (category_rotation_count < NUM_CATEGORY){
        pbf_press_button(context, BUTTON_A, 160ms, 80ms);

        //In sub-menu for item.
        while (true) {
            pbf_press_button(context, BUTTON_A, 160ms, 180ms);

            SelectionArrowWatcher buy_yes_no(
                COLOR_YELLOW, &env.console.overlay(),
                SelectionArrowType::RIGHT,
                { 0.714, 0.599, 0.033, 0.066 }
            );
            FlatWhiteDialogWatcher already_bought(COLOR_RED, &env.console.overlay());
            int ret = wait_until(
                env.console, context,
                std::chrono::seconds(3),
                { buy_yes_no, already_bought }
            );
            switch (ret) {
            case 0:
                env.log("Purchase item.");
                //Purchase Y/N detected.
                pbf_press_button(context, BUTTON_A, 160ms, 500ms);
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();

                //Would you like to wear this out today? (No.)
                pbf_press_button(context, BUTTON_B, 160ms, 500ms);
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();

                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Clothing purchased. Selecting next item."
                );
                break;
            case 1:
                //Item already bought or out of money. Close dialog.
                env.log("Item already bought or out of money.");
                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Item already bought or out of money."
                );
                pbf_press_button(context, BUTTON_A, 160ms, 180ms);
                break;
            default:
                env.log("Error looking for purchase prompt.");
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Error looking for purchase prompt.",
                    env.console
                );
                break;
            }
            context.wait_for_all_requests();

            env.log("Moving on to next item.");
            pbf_press_dpad(context, DPAD_DOWN, 80ms, 180ms);
            context.wait_for_all_requests();

            //Check if we are back at the top of the sub-menu
            SelectionArrowWatcher top_category_item(
                COLOR_YELLOW, &env.console.overlay(),
                SelectionArrowType::RIGHT,
                { 0.034, 0.465, 0.033, 0.071 }
            );
            int retCatTop = wait_until(
                env.console, context,
                1000ms,
                { top_category_item }
            );
            context.wait_for_all_requests();
            if (retCatTop == 0) {
                env.log("Reached top of the item sub-menu.");
                break;
            }
        }

        //Exit sub-menu
        pbf_press_button(context, BUTTON_B, 160ms, 200ms);
        context.wait_for_all_requests();
        
        //Next item on top menu
        env.log("Moving on to next item.");
        pbf_press_dpad(context, DPAD_DOWN, 80ms, 80ms);
        context.wait_for_all_requests();


        //Check if category complete
        SelectionArrowWatcher top_item(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.033, 0.350, 0.033, 0.070}
        );
        int retTop = wait_until(
            env.console, context,
            std::chrono::seconds(1),
            { top_item }
        );
        if (retTop == 0){
            env.log("Reached top of category.");
            if (NUM_CATEGORY > 1){
                env.log("Category rotation set. Moving to next category.");
                pbf_press_button(context, BUTTON_R, 80ms, 100ms);
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

