/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxNatureDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{


void clear_mons_in_front(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    stream.log("Waiting for all " + STRING_POKEMON + " in front of you to get out of the way...");
    WhiteButtonWatcher button(
        COLOR_YELLOW, WhiteButton::ButtonA,
        {0.020, 0.590, 0.035, 0.060},
        WhiteButtonWatcher::FinderType::GONE
    );
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            for (size_t c = 0; c < 40; c++){
                context.wait_for_all_requests();
                context.wait_for(std::chrono::seconds(30));
                stream.log("A " + Pokemon::STRING_POKEMON + " is standing in the way. Whistling and waiting 30 seconds...", COLOR_RED);
                pbf_press_button(context, BUTTON_R, 20, 0);
            }
        },
        {button}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(
            info, stream, "UnableToClearObstacle",
            "Unable to clear " + STRING_POKEMON + " in front of you after 20 min."
        );
    }
#if 0
    WhiteButtonDetector detector(COLOR_RED, WhiteButton::ButtonA, {0.020, 0.590, 0.035, 0.060});
    while (detector.detect(console.video().snapshot())){
        pbf_press_button(context, BUTTON_R, 20, 30 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }
#endif
}




// Call this function when an egg hatching dialog is detected.
// This function presses A to finish the egg hatching dialogs and updates logs and calls callback functions.
// egg_idx: currently which egg in the party is hatching. 0-indexed.
void handle_egg_hatching(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t num_eggs_in_party,
    uint8_t egg_idx,
    std::function<void(uint8_t)> egg_hatched_callback
){
    stream.log("Detect hatching dialog: " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party));
    stream.overlay().add_log("Hatched " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_GREEN);
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            ssf_press_right_joystick(context, 0, 128, 0, 95);
            for(int i = 0; i < 60; i++){
                pbf_mash_button(context, BUTTON_A, 125);
            }
        },
        {overworld}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(
            info, stream, "NoHatchingEnd",
            "hatch_eggs_at_zero_gate(): No end of egg hatching detected after one minute."
        );
    }
    stream.log("Finished hatching animation and dialog.");

    if (egg_hatched_callback){
        egg_hatched_callback(egg_idx);
    }
}

// Assuming on your legendary ride and camera facing the same direction as the player character,
// Turning right to do circullar motion to hatch eggs.
// Function returns when a dialog is detected, meaning an egg is hatching.
// Throw exception when no egg hatching detected after 10 minutes.
void do_egg_cycle_motion(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    AdvanceDialogWatcher dialog(COLOR_RED);
    NormalBattleMenuWatcher battle_menu(COLOR_GREEN);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            // hatch circle:
            // Left joystick forward, right joystick right
            // click left joystick
            ssf_press_left_joystick(context, 128, 0, 0ms, std::chrono::minutes(10));
            ssf_press_right_joystick(context, 255, 128, 0ms, std::chrono::minutes(10));
            pbf_press_button(context, BUTTON_LCLICK, std::chrono::minutes(10), 0ms);
        },
        {
            dialog,
            battle_menu,
        }
    );
    switch (ret){
    case 0:
        break;
    case 1:
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Detected battle menu. You got attacked!",
            stream
        );
        break;
    default:
        dump_image_and_throw_recoverable_exception(
            info, stream, "NoEggToHatch",
            "hatch_eggs_at_zero_gate(): No more egg hatch after 10 minutes."
        );
    }
}

} // anonymous namespace

void order_compote_du_fils(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    // We start this function when we enter the restaurant without pressing any button.

    // Se we first press A to clear a dialog:
    pbf_press_button(context, BUTTON_A, 30, 100);

    bool paid = false;
    bool eating = false;
    while(eating == false){
        context.wait_for_all_requests();

        AdvanceDialogWatcher dialog_watcher(COLOR_RED, DialogType::DIALOG_ALL, std::chrono::milliseconds(100));
        GradientArrowWatcher menu_item_0_watcher(COLOR_BLUE, GradientArrowType::RIGHT, {0.037, 0.224, 0.074, 0.104});
        GradientArrowWatcher menu_item_1_watcher(COLOR_BLUE, GradientArrowType::RIGHT, {0.037, 0.339, 0.074, 0.104});
        PromptDialogWatcher prompt_watcher(COLOR_RED, {0.535, 0.450, 0.367, 0.124});
        
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {dialog_watcher, menu_item_0_watcher, menu_item_1_watcher, prompt_watcher}
        );

        switch (ret){
        case 0:
            stream.log("Detected dialog advance.");
            if (paid){
                // This is a dialog box after we have paid the food.
                // Mash A to clear any remaining dialog before a very long eating animation.
                pbf_mash_button(context, BUTTON_A, 300);
                context.wait_for_all_requests();
                eating = true;
            }else{
                pbf_press_button(context, BUTTON_A, 30, 100);
            }
            break;
        case 1:
            stream.log("Detected restaurant menu.");
            stream.overlay().add_log("Restaurant menu", COLOR_WHITE);
            pbf_press_dpad(context, DPAD_DOWN, 30, 60);
            break;
        case 2:
            stream.log("Detected the dish we want.");
            pbf_press_button(context, BUTTON_A, 30, 100);
            break;
        case 3:
            stream.log("Detected the payment prompt.");
            stream.overlay().add_log("Pay dish", COLOR_WHITE);
            pbf_press_button(context, BUTTON_A, 30, 100);
            paid = true;
            break;
        default:
            dump_image_and_throw_recoverable_exception(
                info, stream, "ErrorOrderFood",
                "order_compote_du_fils(): No recognized state in restaurant menu after 60 seconds."
            );
        }
    } // end state machine for restaurant menu

    { // Now wait for eating animation to finish.
        AdvanceDialogWatcher dialog_watcher(COLOR_RED, DialogType::DIALOG_ALL, std::chrono::milliseconds(100));
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                for(int i = 0; i < 60; i++){
                    pbf_press_button(context, BUTTON_A, 25, 100);
                }
            },
            {{dialog_watcher}}
        );
        if (ret < 0){
            dump_image_and_throw_recoverable_exception(
                info, stream, "EndDiningNotDetected",
                "order_compote_du_fils(): No end of eating after 60 seconds."
            );
        }
    }

    // Now leaving the restaurant
    pbf_mash_button(context, BUTTON_B, 90);
    pbf_wait(context, 100);
    while(true){
        context.wait_for_all_requests();

        AdvanceDialogWatcher dialog_watcher(COLOR_RED, DialogType::DIALOG_ALL, std::chrono::milliseconds(100));
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {dialog_watcher, overworld}
        );
        if (ret == 0){
            pbf_press_button(context, BUTTON_A, 30, 100);
            continue;
        }else if (ret == 1){
            return; // outside restaurant
        }else{
            dump_image_and_throw_recoverable_exception(
                info, stream, "EndLeavingRestaurantNotDetected",
                "order_compote_du_fils(): No end of leaving restaurant after 60 seconds."
            );
        }
    }
}

void picnic_at_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    // Orient camera to look at same direction as player character
    // This is needed because when save-load the game, the camera is reset
    // to this location.
    pbf_press_button(context, BUTTON_L, 50, 40);

    // Move right to make player character facing away from Aera Zero observation station
    pbf_move_left_joystick(context, 255, 32, 50, 50);
    // Press L to move camera to face the same direction as the player character
    pbf_press_button(context, BUTTON_L, 50, 40);
    // Move forward
    pbf_move_left_joystick(context, 128, 0, 125, 0);

    picnic_from_overworld(info, stream, context);
}

bool eat_egg_sandwich_at_picnic(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    EggSandwichType sandwich_type,
    Language language
){
    // Move forward to table to make sandwich
    pbf_move_left_joystick(context, 128, 0, 30, 40);
    context.wait_for_all_requests();

    clear_mons_in_front(env.program_info(), stream, context);
    if (enter_sandwich_recipe_list(env.program_info(), stream, context) == false){
        return false;
    }
    switch (sandwich_type){
    case EggSandwichType::GREAT_PEANUT_BUTTER:
    {
        if (select_sandwich_recipe(env.program_info(), stream, context, 17) == false){
            // cannot find the sandwich recipe, either user has not unlocked it or does not have enough ingredients:
            return false;
        }
        std::map<std::string, uint8_t> fillings = { {"banana", (uint8_t)1} };
        std::vector<std::string> fillings_sorted;
        fillings_sorted.push_back("banana");
        int plates = 1;
        run_sandwich_maker(env, stream, context, language, fillings, fillings_sorted, plates);
        break;
    }
    case EggSandwichType::TWO_SWEET_HERBS:
    case EggSandwichType::SALTY_SWEET_HERBS:
    case EggSandwichType::BITTER_SWEET_HERBS:
        enter_custom_sandwich_mode(env.program_info(), stream, context);
        if (language == Language::None){
            throw UserSetupError(stream.logger(), "Must set game language option to read ingredient lists to make herb sandwich.");
        }
        make_two_herbs_sandwich(
            env,
            stream, context,
            sandwich_type,
            language
        );
        finish_sandwich_eating(env.program_info(), stream, context);
        break;
    default:
        throw InternalProgramError(&stream.logger(), PA_CURRENT_FUNCTION, "Unknown EggSandwichType");
    }

    return true;
}

void collect_eggs_after_sandwich(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t basket_wait_seconds,
    size_t max_eggs,
    size_t& num_eggs_collected,
    std::function<void(size_t new_eggs)> basket_check_callback
){
    context.wait_for_all_requests();
    stream.log("Move past picnic table");
    stream.overlay().add_log("Move past picnic table", COLOR_WHITE);

    //  Recall your ride to reduce obstacles.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);

#if 0
    // this sequence will purposefully fail if Camera support is off. 
    // If we fail to reach the egg basket, we can then check that Camera support is on.

    //  Move forward to table
    pbf_move_left_joystick(context, 128, 0, 80, 40);
    //  Move left
    pbf_move_left_joystick(context, 0, 128, 40, 40);
    //  Move forward to pass table
    pbf_move_left_joystick(context, 128, 0, 80, 40);
    //  Move right
    pbf_move_left_joystick(context, 255, 128, 40, 85);
    //  Move back/right to align to basket
    pbf_move_left_joystick(context, 240, 255, 40, 40);

    //  Move closer to the basket, up to the table
    pbf_press_button(context, BUTTON_L, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 100, 40);

    //  face away from the table
    pbf_press_button(context, BUTTON_L, 20, 105);
    pbf_move_left_joystick(context, 128, 255, 10, 40);  
#endif

#if 1
    // this sequence will work with both Camera Support being Off and On

    //  Move forward to table
    pbf_move_left_joystick(context, 128, 0, 320ms, 480ms);

    //  Move left
    pbf_move_left_joystick(context, 0, 128, 80ms, 480ms);
    pbf_press_button(context, BUTTON_L, 120ms, 480ms);
    pbf_move_left_joystick(context, 128, 0, 320ms, 480ms);

    //  Move forward to pass table
    pbf_move_left_joystick(context, 255, 128, 80ms, 480ms);
    pbf_press_button(context, BUTTON_L, 120ms, 480ms);
    pbf_move_left_joystick(context, 128, 0, 640ms, 480ms);

    //  Move right
    pbf_move_left_joystick(context, 255, 128, 80ms, 480ms);
    pbf_press_button(context, BUTTON_L, 120ms, 480ms);
    pbf_move_left_joystick(context, 128, 0, 320ms, 480ms);

    //  Turn right to face basket
    pbf_move_left_joystick(context, 255, 128, 80ms, 480ms);
    pbf_press_button(context, BUTTON_L, 120ms, 480ms);

    //  Move closer to the basket, up to the table
    pbf_move_left_joystick(context, 128, 0, 800ms, 480ms);

    //  back away from the table, then face forwards towards the basket again
    pbf_move_left_joystick(context, 128, 255, 200ms, 480ms);
    pbf_move_left_joystick(context, 128, 0, 80ms, 480ms);
    pbf_press_button(context, BUTTON_L, 120ms, 480ms);   
#endif

#if 0
    //  Move left
    pbf_move_left_joystick(context, 0, 128, 40, 40);
    //  Move forward to pass table
    pbf_move_left_joystick(context, 128, 0, 80, 40); // old value: 80
    //  Move right
    pbf_move_left_joystick(context, 255, 128, 50, 40);
    //  Move back to face basket
    pbf_move_left_joystick(context, 128, 255, 10, 40);

    //  Move closer to the basket.
    pbf_press_button(context, BUTTON_L, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 10, 40);
#endif


    context.wait_for_all_requests();
    
    const size_t default_collection_interval_seconds = basket_wait_seconds;
    const auto max_egg_wait_time = std::chrono::minutes(30);

    size_t num_checks = 0;
    size_t eggs_collected_cur_session = 0;

    WallClock start = current_time();
    while(true){
        const size_t last_num_eggs_collected = num_eggs_collected;

        clear_mons_in_front(info, stream, context);
        check_basket_to_collect_eggs(info, stream, context, max_eggs, num_eggs_collected);

        const size_t new_eggs_added = num_eggs_collected - last_num_eggs_collected;
        basket_check_callback(new_eggs_added);
        eggs_collected_cur_session += new_eggs_added;
        num_checks++;

        if (num_eggs_collected == max_eggs){
            stream.log("Collected enough eggs: " + std::to_string(max_eggs));
            break;
        }

        if (current_time() - start > max_egg_wait_time){
            stream.log("Picnic time up.");
            stream.overlay().add_log("Picnic time up", COLOR_YELLOW);
            break;
        }
        context.wait_for_all_requests();

        const size_t remaining_eggs = max_eggs - num_eggs_collected;
        const float average_eggs_per_check = eggs_collected_cur_session / (float)num_checks;

        size_t wait_seconds = basket_wait_seconds;
        if (remaining_eggs < average_eggs_per_check){
            // If we have few remaining eggs to hatch, adjust wait time accordingly.
            wait_seconds = size_t(remaining_eggs * default_collection_interval_seconds / average_eggs_per_check);
            stream.log("Last remaining eggs: " + std::to_string(remaining_eggs) + ", avg eggs per check: " +
                std::to_string(average_eggs_per_check) + ", wait secs: " + std::to_string(wait_seconds) + ".");
            if (wait_seconds < 30){
                // Minimum wait period is 30 sec.
                stream.log("Clamp wait time to 30 secs.");
                wait_seconds = 30;
            }
        }
        
        stream.log("Collected " + std::to_string(num_eggs_collected) + " eggs, avg eggs per check: " + std::to_string(average_eggs_per_check)
            + ", wait " + std::to_string(wait_seconds) + " seconds.");
        stream.overlay().add_log("Wait " + std::to_string(wait_seconds) + " secs", COLOR_WHITE);
        
        context.wait_for(std::chrono::seconds(wait_seconds));
    }
}

void check_basket_to_collect_eggs(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t max_eggs, size_t& num_eggs_collected
){
    bool checked = false;
    size_t consecutive_nothing = 0;
    Button last_prompt = BUTTON_NONE;
    bool pending_refuse = false;

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "CheckEggsTimedOut",
                "check_basket_to_collect_eggs(): Still picking up eggs after 5 minutes."
            );
        }

        DialogBoxWatcher picnic(COLOR_RED, false, std::chrono::milliseconds(500));
        AdvanceDialogWatcher advance(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_RED, {0.623, 0.530, 0.243, 0.119});

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
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
            stream.log("Detected no dialog.");
            consecutive_nothing++;
            last_prompt = BUTTON_NONE;
            if (consecutive_nothing >= 10){
                dump_image_and_throw_recoverable_exception(
                    info, stream, "BasketNotFound",
                    "collect_eggs_from_basket(): Basket not found after 10 attempts.\n"
                    "Please make sure that the \"Camera Support\" option is set to \"On\"."
                );
            }
            if (checked){
                stream.log("Done talking to basket.");
                return;
            }
            stream.log("Attempting to talk to basket...");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;

        case 1:
            stream.log("Detected advanced dialog.");
            last_prompt = BUTTON_NONE;
            pbf_press_button(context, BUTTON_B, 20, 30);
            checked = true;
            continue;

        case 2:
            if (last_prompt != 0){
                stream.log("Detected 2nd consecutive prompt. (unexpected)", COLOR_RED);
                //  Repeat the previous button press.
                pbf_press_button(context, last_prompt, 20, 80);
                continue;
            }

            if (pending_refuse){
                stream.log("Confirming refused egg...");
                pbf_press_button(context, BUTTON_A, 20, 80);
                pending_refuse = false;
                continue;
            }

            if (num_eggs_collected < max_eggs){
                num_eggs_collected++;
                
                std::string msg = std::to_string(num_eggs_collected) + "/" + std::to_string(max_eggs);
                stream.log("Found an egg " + msg + ". Keeping...");
                stream.overlay().add_log("Egg " + msg, COLOR_GREEN);
                pbf_press_button(context, BUTTON_A, 20, 80);
                
                last_prompt = BUTTON_A;
            }else{
                stream.log("Found an egg! But we already have enough...");
                stream.overlay().add_log("Full. Skip egg.", COLOR_WHITE);
                pbf_press_button(context, BUTTON_B, 20, 80);
                last_prompt = BUTTON_B;
                pending_refuse = true;
            }
            continue;

        default:
//            dump_image_and_throw_recoverable_exception(
//                info, console, "CheckEggsNoState",
//                "check_basket_to_collect_eggs(): No state detected after 5 seconds."
//            );
            stream.log("Rotating view and trying again...", COLOR_RED);
            pbf_move_right_joystick(context, 0, 128, 30, 0);
        }

    }
}


std::pair<uint8_t, uint8_t> check_egg_party_column(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    BoxEggPartyColumnWatcher egg_column_watcher;
    int ret = wait_until(
        stream, context,
        std::chrono::seconds(10),
        {egg_column_watcher}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(
            info, stream, "CannotReadPartyEggs",
            "check_egg_party_column(): Cannot read party eggs in box system."
        );
    }
    return {egg_column_watcher.num_eggs_found(), egg_column_watcher.num_non_egg_pokemon_found()};
}

uint8_t check_non_eggs_count_in_party(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t expected_non_eggs_count_in_party
){
    auto counts = check_egg_party_column(info, stream, context);
    if (counts.second != expected_non_eggs_count_in_party){
        dump_image_and_throw_recoverable_exception(
            info, stream, "NonEggPokemonInParty",
            "check_non_eggs_count_in_party: Found non-egg pokemon in party"
        );
    }
    return counts.first;
}

void hatch_eggs_at_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback)
{
    bool got_off_ramp = false;
    for(uint8_t egg_idx = 0; egg_idx < num_eggs_in_party; egg_idx++){
        stream.log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party) + ".");
        stream.overlay().add_log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_BLUE);

        // Orient camera to look at same direction as player character
        // This is needed because when save-load the game, the camera is reset
        // to this location.
        pbf_press_button(context, BUTTON_L, 50, 40);

        context.wait_for_all_requests();

        if (got_off_ramp == false){
            AdvanceDialogWatcher dialog(COLOR_RED);
            // first, get off ramp to the empty field for circling motions
            int ret = run_until<ProControllerContext>(
                stream, context,
                [&](ProControllerContext& context){
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
                handle_egg_hatching(info, stream, context, num_eggs_in_party, egg_idx, egg_hatched_callback);
                reset_position_at_zero_gate(info, stream, context);
                continue;
            }

            got_off_ramp = true;
            stream.log("Got off ramp");
        }

        // Circular motions:
        do_egg_cycle_motion(info, stream, context);

        handle_egg_hatching(info, stream, context, num_eggs_in_party, egg_idx, egg_hatched_callback);
    } // end hatching each egg
}

void hatch_eggs_at_area_three_lighthouse(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback)
{
    bool got_off_ramp = false;
    for(uint8_t egg_idx = 0; egg_idx < num_eggs_in_party; egg_idx++){
        stream.log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party) + ".");
        stream.overlay().add_log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_BLUE);

        // Orient camera to look at same direction as player character
        // This is needed because when save-load the game, the camera is reset
        // to this location.
        pbf_press_button(context, BUTTON_L, 50, 40);

        context.wait_for_all_requests();

        if (got_off_ramp == false){
            AdvanceDialogWatcher dialog(COLOR_RED);
            // first, navigate to a clear area in the team star base for circling motions
            int ret = run_until<ProControllerContext>(
                stream, context,
                [&](ProControllerContext& context){
                    if (egg_idx == 0){
                        //Run forward a bit
                        pbf_move_left_joystick(context, 128, 0, 150, 20);
                        //Face at an angle, to avoid the tent to the left
                        pbf_move_left_joystick(context, 0, 0, 50, 50);
                        //Get on your mount
                        pbf_press_button(context, BUTTON_L, 50, 40);
                        pbf_press_button(context, BUTTON_PLUS, 50, 100);
                        //Go in deep, spawns outside the fence like to come in otherwise
                        pbf_move_left_joystick(context, 128, 0, 750, 0);
                        pbf_move_left_joystick(context, 0, 0, 50, 50);
                        pbf_press_button(context, BUTTON_L, 50, 40);
                        pbf_move_left_joystick(context, 128, 0, 550, 0);
                    }
                },
                {dialog}
            );
            if (ret == 0){
                // egg hatching when going off ramp:
                handle_egg_hatching(info, stream, context, num_eggs_in_party, egg_idx, egg_hatched_callback);

                stream.log("Reset location by flying back to lighthouse.");
                // Use map to fly back to the flying spot
                open_map_from_overworld(info, stream, context);
                pbf_move_left_joystick(context, 200, 0, 20, 50);
                fly_to_overworld_from_map(info, stream, context);
                continue;
            }

            got_off_ramp = true;
            stream.log("Got off ramp");
        }

        // Circular motions:
        do_egg_cycle_motion(info, stream, context);

        handle_egg_hatching(info, stream, context, num_eggs_in_party, egg_idx, egg_hatched_callback);
    } // end hatching each egg
}

void hatch_eggs_anywhere(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    bool already_on_ride,
    uint8_t num_eggs_in_party,
    std::function<void(uint8_t)> egg_hatched_callback
){
    if (!already_on_ride){
        // At beginning, ride on Koraidon/Miradon and go off ramp:
        pbf_press_button(context, BUTTON_PLUS, 50, 100);
        context.wait_for_all_requests();
    }

    for(uint8_t egg_idx = 0; egg_idx < num_eggs_in_party; egg_idx++){
        stream.log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party) + ".");
        stream.overlay().add_log("Hatching egg " + std::to_string(egg_idx+1) + "/" + std::to_string(num_eggs_in_party), COLOR_BLUE);

        // Orient camera to look at same direction as player character
        // This is needed because when save-load the game, the camera is reset
        // to this location.
        pbf_press_button(context, BUTTON_L, 50, 40);

        context.wait_for_all_requests();

        // Circular motions:
        do_egg_cycle_motion(info, stream, context);

        handle_egg_hatching(info, stream, context, num_eggs_in_party, egg_idx, egg_hatched_callback);
    } // end hatching each egg
}


void reset_position_at_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    stream.log("Open map and reset location to Zero Gate.");
    // Use map to fly back to the flying spot
    open_map_from_overworld(info, stream, context);

    pbf_move_left_joystick(context, 128, 160, 20, 50);

    fly_to_overworld_from_map(info, stream, context);
}


bool check_baby_info(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    OCR::LanguageOCROption& LANGUAGE,
    Pokemon::StatsHuntIvJudgeFilterTable& FILTERS,
    Pokemon::StatsHuntAction& action
){
    context.wait_for_all_requests();

    Language language = LANGUAGE;
    if (language == Language::None){
        change_view_to_stats_or_judge(stream, context);
    }else{
        change_view_to_judge(stream, context, language);
    }

    VideoOverlaySet overlay_set(stream.overlay());

    BoxShinyWatcher shiny_detector;
    // BoxShinyDetector shiny_detector;
    shiny_detector.make_overlays(overlay_set);
    
    IvJudgeReaderScope iv_reader_scope(stream.overlay(), LANGUAGE);
    BoxGenderDetector gender_detector;
    gender_detector.make_overlays(overlay_set);
    BoxNatureDetector nature_detector(stream.overlay(), LANGUAGE);

    const int shiny_ret = wait_until(
        stream, context,
        std::chrono::milliseconds(500),
        {shiny_detector}
    );
    const bool shiny = (shiny_ret == 0);
    VideoSnapshot screen = stream.video().snapshot();
    
    IvJudgeReader::Results IVs = iv_reader_scope.read(stream.logger(), screen);
    StatsHuntGenderFilter gender = gender_detector.detect(screen);
    NatureReader::Results nature = nature_detector.read(stream.logger(), screen);

    stream.log(IVs.to_string(), COLOR_GREEN);
    stream.log("Gender: " + gender_to_string(gender), COLOR_GREEN);
    stream.log("Nature: " + nature.to_string(), COLOR_GREEN);

    action = FILTERS.get_action(shiny, gender, nature.nature, IVs);

    return shiny;
}

}
}
}

