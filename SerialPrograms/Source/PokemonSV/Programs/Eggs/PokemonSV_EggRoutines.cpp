/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Options/Pokemon_EggHatchFilter.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_EggDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void order_compote_du_fils(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
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
            dump_image_and_throw_recoverable_exception(info, console, "ErrorOrderFood",
                "order_compote_du_fils(): No recognized state in restaurant menu after 60 seconds.");
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
            dump_image_and_throw_recoverable_exception(info, console, "EndDiningNotDetected",
                "order_compote_du_fils(): No end of eating after 60 seconds.");
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
            dump_image_and_throw_recoverable_exception(info, console, "EndLeavingRestaurantNotDetected",
                "order_compote_du_fils(): No end of leaving restaurant after 60 seconds.");
        }
    }
}

void picnic_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    // Orient camera to look at same direction as player character
    // This is needed because when save-load the game, the camera is reset
    // to this location.
    pbf_press_button(context, BUTTON_L, 50, 40);

    // Move right to make player character facing away from Aera Zero observation station
    pbf_move_left_joystick(context, 255, 0, 50, 50);
    // Press L to move camera to face the same direction as the player character
    pbf_press_button(context, BUTTON_L, 50, 40);
    // Move forward
    pbf_move_left_joystick(context, 128, 0, 125, 0);

    picnic_from_overworld(info, console, context);
}

bool eat_egg_sandwich_at_picnic(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context){
    // Move forward to table to make sandwich
    pbf_move_left_joystick(context, 128, 0, 30, 40);
    context.wait_for_all_requests();
    
    bool can_make_sandwich = (
        enter_sandwich_recipe_list(info, console, context) && select_sandwich_recipe(info, console, context, 17)
    );

    if (can_make_sandwich == false){
        return false;
    }

    make_great_peanut_butter_sandwich(info, dispatcher, console, context);
    finish_sandwich_eating(info, console, context);

    return true;
}

void collect_eggs_after_sandwich(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t max_eggs, size_t& num_eggs_collected,
    std::function<void(size_t new_eggs)> basket_check_callback
){
    context.wait_for_all_requests();
    console.log("Move past picnic table");
    console.overlay().add_log("Move past picnic table", COLOR_WHITE);

    // Move left
    pbf_move_left_joystick(context, 0, 128, 40, 40);
    // Move forward to pass table
    pbf_move_left_joystick(context, 128, 0, 80, 40); // old value: 80
    // Move right
    pbf_move_left_joystick(context, 255, 128, 40, 40);
    // Move back to face basket
    pbf_move_left_joystick(context, 128, 255, 10, 40);

    context.wait_for_all_requests();
    
    const auto egg_collection_interval = std::chrono::minutes(3);
    const auto max_egg_wait_time = std::chrono::minutes(30);

    WallClock start = current_time();
    while(true){
        const size_t last_num_eggs_collected = num_eggs_collected;
        check_basket_to_collect_eggs(info, console, context, max_eggs, num_eggs_collected);

        basket_check_callback(num_eggs_collected - last_num_eggs_collected);

        if (num_eggs_collected == max_eggs){
            console.log("Collected enough eggs: " + std::to_string(max_eggs));
            break;
        }

        if (current_time() - start > max_egg_wait_time){
            console.log("Picnic time up.");
            console.overlay().add_log("Picnic time up", COLOR_YELLOW);
            break;
        }
        
        context.wait_for_all_requests();
        console.log("Wait 3 minutes.");
        console.overlay().add_log("Wait 3 min", COLOR_WHITE);
        context.wait_for(egg_collection_interval);
    }
}

void check_basket_to_collect_eggs(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t max_eggs, size_t& num_eggs_collected
){
#if 1
    bool checked = false;
    size_t consecutive_nothing = 0;
    Button last_prompt = 0;
    bool pending_refuse = false;

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "CheckEggsTimedOut",
                "check_basket_to_collect_eggs(): Still picking up eggs after 5 minutes."
            );
        }

        DialogBoxWatcher picnic(COLOR_RED, false, std::chrono::milliseconds(500));
        AdvanceDialogWatcher advance(COLOR_YELLOW);
        PromptDialogWatcher prompt(COLOR_RED, {0.623, 0.530, 0.243, 0.119});

        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(5),
            {
                picnic,
                advance,
                prompt,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));

        if (ret != 0){
            consecutive_nothing = 0;
        }

        switch (ret){
        case 0:
            console.log("Detected no dialog.");
            consecutive_nothing++;
            last_prompt = 0;
            if (consecutive_nothing >= 10){
                dump_image_and_throw_recoverable_exception(
                    info, console, "BasketNotFound",
                    "collect_eggs_from_basket(): Basket not found after 10 attempts."
                );
            }
            if (checked){
                console.log("Done talking to basket.");
                return;
            }
            console.log("Attempting to talk to basket...");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;

        case 1:
            console.log("Detected advanced dialog.");
            last_prompt = 0;
            pbf_press_button(context, BUTTON_B, 20, 30);
            checked = true;
            continue;

        case 2:
            if (last_prompt != 0){
                console.log("Detected 2nd consecutive prompt. (unexpected)", COLOR_RED);
                //  Repeat the previous button press.
                pbf_press_button(context, last_prompt, 20, 80);
                continue;
            }

            if (pending_refuse){
                console.log("Confirming refused egg...");
                pbf_press_button(context, BUTTON_A, 20, 80);
                pending_refuse = false;
                continue;
            }

            if (num_eggs_collected < max_eggs){
                console.log("Found an egg! Keeping...");
                std::string msg = std::to_string(num_eggs_collected) + "/" + std::to_string(max_eggs);
                console.overlay().add_log("Egg " + msg, COLOR_GREEN);
                pbf_press_button(context, BUTTON_A, 20, 80);
                num_eggs_collected++;
                last_prompt = BUTTON_A;
            }else{
                console.log("Found an egg! But we already have enough...");
                console.overlay().add_log("Full. Skip egg.", COLOR_WHITE);
                pbf_press_button(context, BUTTON_B, 20, 80);
                last_prompt = BUTTON_B;
                pending_refuse = true;
            }
            continue;

        default:
            dump_image_and_throw_recoverable_exception(
                info, console, "CheckEggsNoState",
                "check_basket_to_collect_eggs(): No state detected after 5 seconds."
            );
            console.log("Rotating view and trying again...", COLOR_RED);
            pbf_move_right_joystick(context, 0, 128, 30, 0);
        }

    }


#else
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
        dump_image_and_throw_recoverable_exception(info, console, "BasketNotFound",
            "collect_eggs_from_basket(): Basket not found.");
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
            break;
        }
    } // end while true reading basket dialogs

    if (taking_egg){
        dump_image_and_throw_recoverable_exception(info, console, "CollectEggConfirmationNotDetected",
            "check_basket_to_collect_eggs(): Confirmation dialog to of taking egg not detected.");
    } else if (skip_egg_stage > 0){
        dump_image_and_throw_recoverable_exception(info, console, "SkipEggFailure",
            "check_basket_to_collect_eggs(): States of skipping egg timeouts, current state " + std::to_string(skip_egg_stage));
    }
    console.log("Finish talking to basket.");
#endif
}


std::pair<uint8_t, uint8_t> check_egg_party_column(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    EggPartyColumnWatcher egg_column_watcher;
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
        {egg_column_watcher}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(info, console, "CannotReadPartyEggs",
            "check_egg_party_column(): Cannot read party eggs in box system.");
    }
    return {egg_column_watcher.num_eggs_found(), egg_column_watcher.num_non_egg_pokemon_found()};
}

uint8_t check_only_eggs_in_party(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    auto counts = check_egg_party_column(info, console, context);
    if (counts.second > 0){
        dump_image_and_throw_recoverable_exception(info, console, "NonEggPokemonInParty",
            "check_only_eggs_in_party: Found non-egg pokemon in party");
    }
    return counts.first;
}

void hatch_eggs_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t num_eggs_in_party, std::function<void(uint8_t)> egg_hatched_callback)
{
    auto handle_egg_hatching = [&](uint8_t egg_idx){
        console.log("Detect hatching dialog: " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party));
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
            dump_image_and_throw_recoverable_exception(info, console, "NoHatchingEnd",
                "hatch_eggs_at_zero_gate(): No end of egg hatching detected after one minute.");
        }
        console.log("Finished hatching animation and dialog.");

        if (egg_hatched_callback){
            egg_hatched_callback(egg_idx);
        }
    };

    bool got_off_ramp = false;
    for(uint8_t egg_idx = 0; egg_idx < num_eggs_in_party; egg_idx++){
        console.log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party) + ".");
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
                        pbf_move_left_joystick(context, 128, 0, 200, 0);
                    }
                },
                {dialog}
            );
            if (ret == 0){
                // egg hatching when going off ramp:
                handle_egg_hatching(egg_idx);
                reset_position_at_zero_gate(info, console, context);
                continue;
            }

            got_off_ramp = true;
            console.log("Got off ramp");
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
            dump_image_and_throw_recoverable_exception(info, console, "NoEggToHatch",
                "hatch_eggs_at_zero_gate(): No more egg hatch after 10 minutes.");
        }

        handle_egg_hatching(egg_idx);
    } // end hatching each egg
}


void reset_position_at_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Open map and reset location to Zero Gate.");
    // Use map to fly back to the flying spot
    open_map_from_overworld(info, console, context);

    pbf_move_left_joystick(context, 128, 160, 20, 50);

    fly_to_overworld_from_map(info, console, context);
}


bool check_baby_info(ConsoleHandle& console, BotBaseContext& context,
    OCR::LanguageOCROption& LANGUAGE, Pokemon::EggHatchFilterTable& FILTERS,
    Pokemon::EggHatchAction& action
){
    context.wait_for_all_requests();
    auto screen = console.video().snapshot();

    VideoOverlaySet overlay_set(console.overlay());

    BoxShinyDetector shiny_detector;
    shiny_detector.make_overlays(overlay_set);
    IVCheckerReaderScope iv_reader_scope(console.overlay(), LANGUAGE);
    BoxGenderDetector gender_detector;
    gender_detector.make_overlays(overlay_set);

    const bool shiny = shiny_detector.detect(screen);

    IVCheckerReader::Results IVs = iv_reader_scope.read(console.logger(), screen);
    EggHatchGenderFilter gender = gender_detector.detect(screen);

    console.log(IVs.to_string(), COLOR_GREEN);
    console.log("Gender: " + gender_to_string(gender), COLOR_GREEN);

    action = FILTERS.get_action(shiny, IVs, gender);

    return shiny;
}





}
}
}
