/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void order_compote_du_fils(ConsoleHandle& console, BotBaseContext& context){
    // We start this function when we enter the restaurant without pressing any button.

    // Se we first press A to clear a dialog:
    pbf_press_button(context, BUTTON_A, 30, 100);

    bool paid = false;
    bool eating = false;
    while(eating == false){
        context.wait_for_all_requests();

        AdvanceDialogWatcher dialog_watcher(COLOR_RED, std::chrono::milliseconds(100));
        GradientArrowWatcher menu_item_0_watcher(COLOR_BLUE, GradientArrowType::RIGHT, {0.037, 0.224, 0.074, 0.104});
        GradientArrowWatcher menu_item_1_watcher(COLOR_BLUE, GradientArrowType::RIGHT, {0.037, 0.339, 0.074, 0.104});
        PromptDialogWatcher prompt_watcher(COLOR_RED, {0.535, 0.450, 0.367, 0.124});
        
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {dialog_watcher, menu_item_0_watcher, menu_item_1_watcher, prompt_watcher}
        );

        switch (ret){
        case 0:
            console.log("Detected dialog advance.");
            if (paid){
                // This is a dialog box after we have paid the food.
                // Mash A to clear any remaining dialog before a very long eating animation.
                pbf_mash_button(context, BUTTON_A, 300);
                context.wait_for_all_requests();
                eating = true;
            } else{
                pbf_press_button(context, BUTTON_A, 30, 100);
            }
            break;
        case 1:
            console.log("Detected restaurant menu.");
            console.overlay().add_log("Restaurant menu", COLOR_WHITE);
            pbf_press_dpad(context, DPAD_DOWN, 30, 60);
            break;
        case 2:
            console.log("Detected the dish we want.");
            pbf_press_button(context, BUTTON_A, 30, 100);
            break;
        case 3:
            console.log("Detected the payment prompt.");
            console.overlay().add_log("Pay dish", COLOR_WHITE);
            pbf_press_button(context, BUTTON_A, 30, 100);
            paid = true;
            break;
        default:
            throw OperationFailedException(console.logger(), "order_compote_du_fils(): No recognized state in restaurant menu after 60 seconds.");
        }
    } // end state machine for restaurant menu

    { // Now wait for eating animation to finish.
        AdvanceDialogWatcher dialog_watcher(COLOR_RED, std::chrono::milliseconds(100));
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for(int i = 0; i < 60; i++){
                    pbf_press_button(context, BUTTON_A, 25, 100);
                }
            },
            {{dialog_watcher}}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "order_compote_du_fils(): No end of eating after 60 seconds.");
        }
    }

    // Now leaving the restaurant
    pbf_mash_button(context, BUTTON_B, 90);
    pbf_wait(context, 100);
    while(true){
        context.wait_for_all_requests();

        AdvanceDialogWatcher dialog_watcher(COLOR_RED, std::chrono::milliseconds(100));
        OverworldWatcher overworld(COLOR_CYAN);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {dialog_watcher, overworld}
        );
        if (ret == 0){
            pbf_press_button(context, BUTTON_A, 30, 100);
            continue;
        }
        else if (ret == 1){
            return; // outside restaurant
        }
        else{
            throw OperationFailedException(console.logger(), "order_compote_du_fils(): No end of leaving restaurant after 60 seconds.");
        }
    }
}


void collect_eggs_from_basket(ConsoleHandle& console, BotBaseContext& context, size_t max_eggs, size_t& num_eggs_collected){
    pbf_press_button(context, BUTTON_A, 20, 80);

    bool basket_found = false;
    for(int i = 0; i < 5; i++){
        context.wait_for_all_requests();
        AdvanceDialogWatcher dialog_watcher(COLOR_RED, std::chrono::milliseconds(100));
        int ret = wait_until(
            console, context,
            std::chrono::seconds(3),
            {dialog_watcher}
        );
        if (ret == 0){
            console.log("Found basket.");
            console.overlay().add_log("Found basket", COLOR_WHITE);
            // found dialog, so we are interacting with a basket
            basket_found = true;
            break;
        }

        // no dialog found, maybe the game dropped the button A press? Try again
        pbf_press_button(context, BUTTON_A, 20, 80);
    }

    if (basket_found == false){
        throw OperationFailedException(console.logger(), "collect_eggs_from_basket(): Basket not found.");
    }

    // Press A to clear the current dialog
    pbf_press_button(context, BUTTON_A, 20, 80);
    bool taking_egg = false;
    // skipping egg is quite complex, with several stages:
    // stage 0: no egg skipping
    // stage 1: press B to not collect eggs, waiting for game to show text of "sending to academy".
    // stage 2: press A to show the yes/no prompt for "sending to academy", waiting for selection on prompt
    // stage 3: press A to confirm sending to academy, waiting for next advance dialog
    int skip_egg_stage = 0;
    while (true) {
        context.wait_for_all_requests();

        AdvanceDialogWatcher advance(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_RED, {0.623, 0.530, 0.243, 0.119});
        int ret = wait_until(
            console, context,
            std::chrono::seconds(5),
            {advance, prompt}
        );
        if (ret == 0){
            console.log("Detected dialog box.");
            if (taking_egg){
                num_eggs_collected++;
                std::string msg = std::to_string(num_eggs_collected) + "/" + std::to_string(max_eggs);
                console.log("Egg taken. " + msg);
                console.overlay().add_log("Egg " + msg, COLOR_GREEN);
                taking_egg = false;
            }
            else if (skip_egg_stage == 1){
                console.log("Enter skip egg stage 2.");
                skip_egg_stage = 2;
            }
            else if (skip_egg_stage == 2){
                console.log("Drop button? Still in skip egg stage 2.");
            }
            else if (skip_egg_stage == 3){
                console.log("Skip egg all stage complete.");
                console.overlay().add_log("Full. Skip egg.", COLOR_WHITE);
                skip_egg_stage = 0;
            }
            pbf_press_button(context, BUTTON_A, 20, 80);
        }
        else if (ret == 1){
            console.log("Detected prompt.");
            if (skip_egg_stage == 1){
                console.log("Try to skip egg, still in stage 1");
                pbf_press_button(context, BUTTON_B, 20, 80);
            }
            else if (skip_egg_stage == 2){
                console.log("Confirm skip egg. Enter stage 3.");
                skip_egg_stage = 3;
                pbf_press_button(context, BUTTON_A, 20, 80);
            }
            else if (skip_egg_stage == 3){
                console.log("Drop button? Still in stage 3.");
                pbf_press_button(context, BUTTON_A, 20, 80);
            }
            else if (num_eggs_collected == max_eggs){
                console.log("Try to skip egg, enter stage 1");
                skip_egg_stage = 1;
                pbf_press_button(context, BUTTON_B, 20, 80);
            } else {
                console.log("Try to take egg");
                taking_egg = true;
                pbf_press_button(context, BUTTON_A, 20, 80);
            }
        }
        else { // No more dialog, the basket checking ends
            return;
        }
    } // end while true reading basket dialogs
}


void make_great_peanut_butter_sandwich(ConsoleHandle& console, BotBaseContext& context){
    
}

}
}
}
