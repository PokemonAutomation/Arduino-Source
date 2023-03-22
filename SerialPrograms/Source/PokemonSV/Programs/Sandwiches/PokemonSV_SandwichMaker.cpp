/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV/Resources/PokemonSV_FillingsCoordinates.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV_SandwichMaker.h"

//#include <iostream>
//using std::cout;
//using std::endl;
#include <unordered_map>
#include <algorithm>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

SandwichMaker_Descriptor::SandwichMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:SandwichMaker",
        STRING_POKEMON + " SV", "Sandwich Maker",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/SandwichMaker.md",
        "Make a sandwich of your choice.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

SandwichMaker::SandwichMaker()
    : GO_HOME_WHEN_DONE(false)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(SANDWICH_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void SandwichMaker::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    assert_16_9_720p_min(env.logger(), env.console);

    if (SANDWICH_OPTIONS.LANGUAGE == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game langauge option to read ingredient lists.");
    }

    int num_fillings = 0;
    int num_condiments = 0;
    std::map<std::string, uint8_t> fillings;
    std::map<std::string, uint8_t> condiments;

    //Add the selected ingredients to the maps if set to custom
    if (SANDWICH_OPTIONS.SANDWICH_RECIPE == SandwichMakerOption::SandwichRecipe::custom) {
        env.log("Custom sandwich selected. Validating ingredients.", COLOR_BLACK);
        env.console.overlay().add_log("Custom sandwich selected. Validating ingredients.", COLOR_WHITE);

        std::vector<std::unique_ptr<SandwichIngredientsTableRow>> table = SANDWICH_OPTIONS.SANDWICH_INGREDIENTS.copy_snapshot();

        for (const std::unique_ptr<SandwichIngredientsTableRow>& row : table) {
            std::string table_item = row->item.slug();
            if (!(table_item == "baguette")) { //ignore baguette
                if (std::find(ALL_SANDWICH_FILLINGS_SLUGS().begin(), ALL_SANDWICH_FILLINGS_SLUGS().end(), table_item) != ALL_SANDWICH_FILLINGS_SLUGS().end()) {
                    if (fillings.find(table_item) == fillings.end()) {
                        fillings.insert(make_pair(table_item, 0));
                    }
                    fillings[table_item]++;
                    num_fillings++;
                }
                else {
                    if (condiments.find(table_item) == condiments.end()) {
                        condiments.insert(make_pair(table_item, 0));
                    }
                    condiments[table_item]++;
                    num_condiments++;
                }
            }
            else {
                env.log("Skipping baguette as it is unobtainable.");
                env.console.overlay().add_log("Skipping baguette as it is unobtainable.", COLOR_WHITE);
            }
        }

        if (num_fillings == 0 || num_condiments == 0) {
            throw UserSetupError(env.console.logger(), "Must have at least one filling and at least one condiment.");
        }

        if (num_fillings > 6 || num_condiments > 4) {
            throw UserSetupError(env.console.logger(), "Number of fillings exceed 6 and/or number of condiments exceed 4.");
        }
        env.log("Ingredients validated.", COLOR_BLACK);
        env.console.overlay().add_log("Ingredients validated.", COLOR_WHITE);
    }
    //Otherwise get the preset ingredients
    else {
        env.log("Preset sandwich selected.", COLOR_BLACK);
        env.console.overlay().add_log("Preset sandwich selected.", COLOR_WHITE);

        std::vector<std::string> table = SANDWICH_OPTIONS.get_premade_ingredients(SANDWICH_OPTIONS.SANDWICH_RECIPE);

        for (auto&& s : table) {
            if (std::find(ALL_SANDWICH_FILLINGS_SLUGS().begin(), ALL_SANDWICH_FILLINGS_SLUGS().end(), s) != ALL_SANDWICH_FILLINGS_SLUGS().end()) {
                if (fillings.find(s) == fillings.end()) {
                    fillings.insert(make_pair(s, 0));
                }
                fillings[s]++;
                num_fillings++;
            }
            else {
                if (condiments.find(s) == condiments.end()) {
                    condiments.insert(make_pair(s, 0));
                }
                condiments[s]++;
                num_condiments++;
            }
        }
    }

    /*
    //Print ingredients
    cout << "Fillings:" << endl;
    for (const auto& [key, value] : fillings) {
        std::cout << key << ": " << (int)value << endl;
    }
    cout << "Condiments:" << endl;
    for (const auto& [key, value] : condiments) {
        std::cout << key << ": " << (int)value << endl;
    }
    */

    //Sort the fillings by priority for building (ex. large items on bottom, cherry tomatoes on top)
    //std::vector<std::string> fillings_game_order = {"lettuce", "tomato", "cherry-tomatoes", "cucumber", "pickle", "onion", "red-onion", "green-bell-pepper", "red-bell-pepper",
    //    "yellow-bell-pepper", "avocado", "bacon", "ham", "prosciutto", "chorizo", "herbed-sausage", "hamburger", "klawf-stick", "smoked-fillet", "fried-fillet", "egg", "potato-tortilla",
    //    "tofu", "rice", "noodles", "potato-salad", "cheese", "banana", "strawberry", "apple", "kiwi", "pineapple", "jalapeno", "watercress", "basil"};
    std::vector<std::string> fillings_game_order = { "hamburger", "rice", "noodles", "potato-salad", "smoked-fillet", "fried-fillet", "tofu", "chorizo", "herbed-sausage",
        "potato-tortilla", "klawf-stick", "lettuce", "tomato", "cucumber", "pickle", "onion", "red-onion", "green-bell-pepper", "red-bell-pepper", "yellow-bell-pepper", "avocado",
        "bacon", "ham", "prosciutto", "cheese", "banana", "strawberry", "apple", "kiwi", "pineapple", "jalape\xc3\xb1o", "watercress", "egg", "basil", "cherry-tomatoes" };

    //Add keys to new vector and sort
    std::vector<std::string> fillings_sorted;
    for (auto i = fillings.begin(); i != fillings.end(); i++) {
        fillings_sorted.push_back(i->first);
    }
    std::unordered_map<std::string, int> temp_map;
    for (auto i = 0; i < (int)fillings_game_order.size(); i++) {
        temp_map[fillings_game_order[i]] = i;
    }
    auto compare = [&temp_map](const std::string& s, const std::string& s1) {
        return temp_map[s] < temp_map[s1];
    };
    std::sort(fillings_sorted.begin(), fillings_sorted.end(), compare);

    /*
    //Print sorted fillings
    cout << "Sorted fillings:" << endl;
    for (auto i : fillings_sorted) {
        cout << i << endl;
    }
    */

    //Calculate number of bowls there will be on the build screen
    //Get each ingredient in list order, then get the number of times it appears from the map
    //Also store how many of each ingredient is in each bowl (ex. 6 onion in first bowl and then 3 onion in the next)
    int bowls = 0;
    std::vector<int> bowl_amounts;

    for (auto i : fillings_sorted) {
        FillingsCoordinates ingreed;

        //Add "full" bowls
        int bowl_calcs = (int)(fillings[i] / ingreed.get_filling_information(i).servingsPerBowl);
        if (bowl_calcs != 0) {
            bowls += bowl_calcs;
            for (int j = 0; j < bowl_calcs; j++) {
                bowl_amounts.push_back((int)(ingreed.get_filling_information(i).servingsPerBowl)* (int)(ingreed.get_filling_information(i).piecesPerServing));
            }
        }

        //Add bowls for remaining servings
        int bowl_remaining = ((int)(fillings[i] % ingreed.get_filling_information(i).servingsPerBowl));
        if (bowl_remaining != 0) {
            bowls++;
            bowl_amounts.push_back(bowl_remaining * (int)(ingreed.get_filling_information(i).piecesPerServing));
        }
    }
    //cout << "Number of bowls: " << bowls << endl;
    env.log("Number of bowls: " + std::to_string(bowls), COLOR_BLACK);
    env.console.overlay().add_log("Number of bowls: " + std::to_string(bowls), COLOR_WHITE);

    //Player must be on default sandwich menu
    std::map<std::string, uint8_t> fillings_copy(fillings); //Making a copy as we need the map for later
    enter_custom_sandwich_mode(env.program_info(), env.console, context);
    add_sandwich_ingredients(env.realtime_dispatcher(), env.console, context, SANDWICH_OPTIONS.LANGUAGE, std::move(fillings_copy), std::move(condiments));
    wait_for_initial_hand(env.program_info(), env.console, context);

    //Wait for labels to appear
    env.log("Waiting for labels to appear.", COLOR_BLACK);
    env.console.overlay().add_log("Waiting for labels to appear.", COLOR_WHITE);
    pbf_wait(context, 300);
    context.wait_for_all_requests();

    //Now read in bowl labels and store which bowl has what
    //TODO: Clean this up - this is a mess
    env.log("Reading bowl labels.", COLOR_BLACK);
    env.console.overlay().add_log("Reading bowl labels.", COLOR_WHITE);

    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox left_bowl_label(0.099, 0.270, 0.205, 0.041);
    ImageFloatBox center_bowl_label(0.397, 0.268, 0.203, 0.044);
    ImageFloatBox right_bowl_label(0.699, 0.269, 0.201, 0.044);

    //VideoOverlaySet label_set(env.console);
    //label_set.add(COLOR_BLUE, center_bowl_label);
    std::vector<std::string> bowl_order;

    //Get 3 default labels
    ImageRGB32 image_center_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, center_bowl_label),
        combine_rgb(215, 215, 215), combine_rgb(255, 255, 255), true
    );
    ImageRGB32 image_left_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, left_bowl_label),
        combine_rgb(215, 215, 215), combine_rgb(255, 255, 255), true
    );
    ImageRGB32 image_right_label = to_blackwhite_rgb32_range(
        extract_box_reference(screen, right_bowl_label),
        combine_rgb(215, 215, 215), combine_rgb(255, 255, 255), true
    );

    OCR::StringMatchResult result = PokemonSV::SandwichFillingOCR::instance().read_substring(
        env.console, SANDWICH_OPTIONS.LANGUAGE, image_center_label,
        OCR::BLACK_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
    result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
    if (result.results.empty()) {
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "No ingredient found on center label.",
            true
        );
    }
    for (auto& r : result.results) {
        env.console.log("Ingredient found on center label: " + r.second.token);
        env.console.overlay().add_log("Ingredient found on center label : " + r.second.token, COLOR_WHITE);
        bowl_order.push_back(r.second.token);
    }
    //Get left (2nd) ingredient
    result = PokemonSV::SandwichFillingOCR::instance().read_substring(
        env.console, SANDWICH_OPTIONS.LANGUAGE, image_left_label,
        OCR::BLACK_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
    result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
    if (result.results.empty()) {
        env.log("No ingredient found on left label.", COLOR_BLACK);
        env.console.overlay().add_log("No ingredient found on left label.", COLOR_WHITE);
    }
    for (auto& r : result.results) {
        env.console.log("Ingredient found on left label: " + r.second.token);
        env.console.overlay().add_log("Ingredient found on left label: " + r.second.token, COLOR_WHITE);
        bowl_order.push_back(r.second.token);
    }
    //Get right (3rd) ingredient
    result = PokemonSV::SandwichFillingOCR::instance().read_substring(
        env.console, SANDWICH_OPTIONS.LANGUAGE, image_right_label,
        OCR::BLACK_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
    result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
    if (result.results.empty()) {
        env.log("No ingredient found on right label.", COLOR_BLACK);
        env.console.overlay().add_log("No ingredient found on right label.", COLOR_WHITE);
    }
    for (auto& r : result.results) {
        env.console.log("Ingredient found on right label: " + r.second.token);
        env.console.overlay().add_log("Ingredient found on right label: " + r.second.token, COLOR_WHITE);
        bowl_order.push_back(r.second.token);
    }
    //Get remaining ingredients if any
    //center 1, left 2, right 3, far left 4, far far left/right 5, right 6
    //this differs from the game layout: far right is 5 and far far left/right is 6 in game
    //however as long as we stay internally consistent with this numbering it will work
    for (int i = 0; i < (bowls - 3); i++) {
        pbf_press_button(context, BUTTON_R, 20, 80);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        VideoSnapshot screen2 = env.console.video().snapshot();
        ImageRGB32 image_side_label = to_blackwhite_rgb32_range(
            extract_box_reference(screen2, left_bowl_label),
            combine_rgb(215, 215, 215), combine_rgb(255, 255, 255), true
        );
        //image_side_label.save("./image_side_label.png");

        result = PokemonSV::SandwichFillingOCR::instance().read_substring(
            env.console, SANDWICH_OPTIONS.LANGUAGE, image_side_label,
            OCR::BLACK_TEXT_FILTERS()
        );
        result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
        result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
        if (result.results.empty()) {
            env.log("No ingredient found on side label.", COLOR_BLACK);
            env.console.overlay().add_log("No ingredient found on side label.", COLOR_WHITE);
        }
        for (auto& r : result.results) {
            env.console.log("Ingredient found on side label: " + r.second.token);
            env.console.overlay().add_log("Ingredient found on side label: " + r.second.token, COLOR_WHITE);
            bowl_order.push_back(r.second.token);
        }
    }

    //Now re-center bowls
    env.log("Re-centering bowls if needed.", COLOR_BLACK);
    env.console.overlay().add_log("Re-centering bowls if needed.", COLOR_WHITE);
    for (int i = 0; i < (bowls - 3); i++) {
        pbf_press_button(context, BUTTON_L, 20, 80);
    }

    /*
    cout << "Labels:" << endl;
    for (auto i : bowl_order) {
        cout << i << endl;
    }
    */

    //If a label fails to read it'll cause issues down the line
    if ((int)bowl_order.size() != bowls) {
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "Number of bowl labels did not match number of bowls.",
            true
        );
    }

    //Finally.
    env.log("Start making sandwich", COLOR_BLACK);
    env.console.overlay().add_log("Start making sandwich.", COLOR_WHITE);

    const ImageFloatBox HAND_INITIAL_BOX{ 0.440, 0.455, 0.112, 0.179 };
    const ImageFloatBox center_bowl{ 0.455, 0.130, 0.090, 0.030 };
    const ImageFloatBox left_bowl{ 0.190, 0.136, 0.096, 0.031 };
    const ImageFloatBox right_bowl{ 0.715, 0.140, 0.108, 0.033 };

    ImageFloatBox target_bowl = center_bowl;
    //Initial position handling
    auto end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::FREE, false, HAND_INITIAL_BOX, HAND_INITIAL_BOX);
    move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, true, { 0, 0, 1.0, 1.0 }, HAND_INITIAL_BOX);
    context.wait_for_all_requests();

    //Find fillings and add them in order
    for (auto i : fillings_sorted) {
        FillingsCoordinates ingreed;
        //cout << "Placing " << i << endl;
        env.console.overlay().add_log("Placing " + i, COLOR_WHITE);

        int times_to_place = (int)(ingreed.get_filling_information(i).piecesPerServing) * (fillings.find(i)->second);
        int placement_number = 0;

        //cout << "Times to place: " << times_to_place << endl;
        env.console.overlay().add_log("Times to place: " + std::to_string(times_to_place), COLOR_WHITE);

        std::vector<int> bowl_index;
        //Get the bowls we want to go to
        for (int j = 0; j < (int)bowl_order.size(); j++) {
            if (i == bowl_order.at(j)) {
                bowl_index.push_back(j);
            }
        }

        //Target the correct filling bowl and place until it is empty
        for (int j = 0; j < (int)bowl_index.size(); j++) {
            //Navigate to bowl and set target bowl
            //cout << "Target bowl: " << bowl_index.at(j) << endl;
            env.console.overlay().add_log("Target bowl: " + std::to_string(bowl_index.at(j)), COLOR_WHITE);
            switch (bowl_index.at(j)) {
            case 0:
                target_bowl = center_bowl;
                break;
            case 1:
                target_bowl = left_bowl;
                break;
            case 2:
                target_bowl = right_bowl;
                break;
            case 3: case 4: case 5: case 6:
                //Press R the appropriate number of times
                for (int k = 2; k < bowl_index.at(j); k++) {
                    pbf_press_button(context, BUTTON_R, 20, 80);
                }
                target_bowl = left_bowl;
                break;
            default:
                break;
            }

            //Place the fillings until label does not light up yellow on grab/the piece count is not hit
            while (true) {
                //Break out after placing all pieces of the filling
                if (placement_number == times_to_place) {
                    break;
                }

                end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::FREE, false, { 0, 0, 1.0, 1.0 }, target_bowl);
                context.wait_for_all_requests();

                //Get placement location
                ImageFloatBox placement_target = ingreed.get_filling_information(i).placementCoordinates.at((int)fillings.find(i)->second).at(placement_number);

                end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, true, expand_box(end_box), placement_target);
                context.wait_for_all_requests();

                //If any of the labels are yellow continue. Otherwise assume bowl is empty move on to the next.
                VideoSnapshot label_color_check = env.console.video().snapshot();
                ImageRGB32 left_check = filter_rgb32_range(
                    extract_box_reference(label_color_check, left_bowl_label),
                    combine_rgb(180, 161, 0), combine_rgb(255, 255, 100), Color(0), false
                );
                ImageRGB32 right_check = filter_rgb32_range(
                    extract_box_reference(label_color_check, right_bowl_label),
                    combine_rgb(180, 161, 0), combine_rgb(255, 255, 100), Color(0), false
                );
                ImageRGB32 center_check = filter_rgb32_range(
                    extract_box_reference(label_color_check, center_bowl_label),
                    combine_rgb(180, 161, 0), combine_rgb(255, 255, 100), Color(0), false
                );
                ImageStats left_stats = image_stats(left_check);
                ImageStats right_stats = image_stats(right_check);
                ImageStats center_stats = image_stats(center_check);

                //cout << "Left stats: " << left_stats.count << endl;
                //cout << "Right stats: " << right_stats.count << endl;
                //cout << "Center stats: " << center_stats.count << endl;

                //The label check is needed for ingredients with multiple bowls as we don't know which bowl has what amount
                if (left_stats.count < 100 && right_stats.count < 100 && center_stats.count < 100) {
                    context.wait_for_all_requests();
                    break;
                }

                //If the bowl is empty the increment is skipped using the above break
                placement_number++;
            }

            //Reset bowl positions
            for (int k = 2; k < bowl_index.at(j); k++) {
                pbf_press_button(context, BUTTON_L, 20, 80);
            }
        }
    }
    // Handle top slice by tossing it away
    SandwichHandWatcher grabbing_hand(SandwichHandType::FREE, {0, 0, 1.0, 1.0});
    int ret = wait_until(env.console, context, std::chrono::seconds(30), { grabbing_hand });
    if (ret < 0) {
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "SandwichMaker: Cannot detect grabing hand when waiting for upper bread.",
            grabbing_hand.last_snapshot()
        );
    }

    auto hand_box = hand_location_to_box(grabbing_hand.location());

    end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, false, expand_box(hand_box), center_bowl);
    pbf_mash_button(context, BUTTON_A, 125 * 5);

    env.log("Hand end box " + box_to_string(end_box));
    env.log("Built sandwich", COLOR_BLACK);
    env.console.overlay().add_log("Hand end box " + box_to_string(end_box), COLOR_WHITE);
    env.console.overlay().add_log("Built sandwich.", COLOR_WHITE);
    context.wait_for_all_requests();

    finish_sandwich_eating(env.program_info(), env.console, context);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

