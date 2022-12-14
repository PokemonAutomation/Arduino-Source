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
#include "PokemonSV/Inference/PokemonSV_EggDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
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


std::pair<uint8_t, uint8_t> check_egg_party_column(ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    EggPartyColumnWatcher egg_column_watcher;
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
        {egg_column_watcher}
    );
    if (ret < 0){
        throw OperationFailedException(console.logger(), "check_egg_party_column(): Cannot read party eggs in box system.");
    }
    return {egg_column_watcher.num_eggs_found(), egg_column_watcher.num_non_egg_pokemon_found()};
}

void hatch_eggs_at_zero_gate(ConsoleHandle& console, BotBaseContext& context,
    uint8_t num_eggs_in_party, std::function<void(uint8_t)> egg_hatched_callback)
{
    auto handle_egg_hatching = [&](uint8_t egg_idx){
        console.overlay().add_log("Hatched " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_GREEN);
        OverworldWatcher overworld(COLOR_CYAN);
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                for(int i = 0; i < 60; i++){
                    pbf_mash_button(context, BUTTON_A, 125);
                }
            },
            {overworld}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "hatch_eggs_at_zero_gate(): No end of egg hatching detected after one minute.");
        }

        if (egg_hatched_callback){
            egg_hatched_callback(egg_idx);
        }
    };

    bool got_off_ramp = false;
    for(uint8_t egg_idx = 0; egg_idx < num_eggs_in_party; egg_idx++){
        console.overlay().add_log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_BLUE);

        // Orient camera to look at same direction as player character
        // This is needed because when save-load the game, the camera is reset
        // to this location.
        pbf_press_button(context, BUTTON_L, 50, 40);

        context.wait_for_all_requests();

        AdvanceDialogWatcher dialog(COLOR_RED);

        int ret = 0;
        if (got_off_ramp == false){
            // first, get off ramp to the empty field for circling motions
            ret = run_until(
                console, context,
                [&](BotBaseContext& context){
                    if (egg_idx == 0){
                        // At beginning, ride on Koraidon/Miradon and go off ramp:
                        pbf_press_button(context, BUTTON_PLUS, 50, 100);
                        // Move right to make player character facing away from Aera Zero observation station
                        pbf_move_left_joystick(context, 255, 0, 50, 50);
                        // Press L to move camera to face the same direction as the player character
                        pbf_press_button(context, BUTTON_L, 50, 40);
                        // Move forward
                        pbf_move_left_joystick(context, 128, 0, 350, 0);
                    }
                },
                {dialog}
            );
            if (ret == 0){
                // egg hatching when going off ramp:
                handle_egg_hatching(egg_idx);
                reset_position_at_zero_gate(console, context);
                continue;
            }

            got_off_ramp = true;
        }

        // Circular motions:
        ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                // hatch circle:
                // Left joystick forward, right joystick right
                // click left joystick
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE,
                    128, 0, 255, 128, 20);
                pbf_controller_state(context, 0, DPAD_NONE, 128, 0, 255, 128, 20);
                for(int j = 0; j < 600; j++){
                    pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE,
                        128, 0, 255, 128, TICKS_PER_SECOND);
                }
            },
            {dialog}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "hatch_eggs_at_zero_gate(): No more egg hatch after 10 minutes.");
        }

        handle_egg_hatching(egg_idx);
    } // end hatching each egg
}


void reset_position_at_zero_gate(ConsoleHandle& console, BotBaseContext& context){
    // Use map to fly back to the flying spot
    open_map_from_overworld(console, context);

    pbf_move_left_joystick(context, 128, 160, 20, 50);

    fly_to_overworld_from_map(console, context);
}


}
}
}
