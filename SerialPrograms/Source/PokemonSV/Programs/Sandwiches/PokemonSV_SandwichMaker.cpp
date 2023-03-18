/*  Sandwich Maker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Common/Cpp/Exceptions.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV/Resources/PokemonSV_FillingsCoordinates.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV_SandwichMaker.h"

#include <iostream>
using std::cout;
using std::endl;
#include <unordered_map>

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
            }
        }

        if (num_fillings == 0 || num_condiments == 0) {
            throw UserSetupError(env.console.logger(), "Must have at least one filling and at least one condiment.");
        }

        if (num_fillings > 6 || num_condiments > 4) {
            throw UserSetupError(env.console.logger(), "Number of fillings exceed 6 and/or number of condiments exceed 4.");
        }
        env.log("Ingredients validated.", COLOR_BLACK);
    }
    //Otherwise get the preset ingredients
    else {
        env.log("Preset sandwich selected.", COLOR_BLACK);

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

    //Print ingredients
    cout << "Fillings:" << endl;
    for (const auto& [key, value] : fillings) {
        std::cout << key << ": " << (int)value << endl;
    }
    cout << "Condiments:" << endl;
    for (const auto& [key, value] : condiments) {
        std::cout << key << ": " << (int)value << endl;
    }

    //Sort the fillings to keep the bowls on the build screen in a consistent order
    //Ex. Adding ingredients in the order of lettuce, tomato, cherry tomato, cucumber will result in their respective positions being center, left, right, far left
    //But adding the ingredients in the order of tomato, cucumber, lettuce, cherry tomato will result in their respective positions being left, far left, center, right
    //So lettuce will always be center, tomato will always be left, etc.
    //This is so when we are placing the filling we will know which bowl to go to and how many ingredients to expect

    //Is this supposed to be alphabetical? the order is off when selecting in the minigame
    std::vector<std::string> fillings_game_order = {"lettuce", "tomato", "cherry-tomatoes", "cucumber", "pickle", "onion", "red-onion", "green-bell-pepper", "red-bell-pepper",
        "yellow-bell-pepper", "avocado", "bacon", "ham", "prosciutto", "chorizo", "herbed-sausage", "hamburger", "klawf-stick", "smoked-fillet", "fried-fillet", "egg", "potato-tortilla",
        "tofu", "rice", "noodles", "potato-salad", "cheese", "banana", "strawberry", "apple", "kiwi", "pineapple", "jalapeno", "watercress", "basil"};
    //Add keys to new vector and sort
    std::vector<std::string> fillings_sorted;
    for (auto i = fillings.begin(); i != fillings.end(); i++) {
        fillings_sorted.push_back(i->first);
    }
    std::unordered_map<std::string, int> temp_map;
    for (auto i = 0; i < fillings_game_order.size(); i++) {
        temp_map[fillings_game_order[i]] = i;
    }
    auto compare = [&temp_map](const std::string& s, const std::string& s1) {
        return temp_map[s] < temp_map[s1];
    };
    std::sort(fillings_sorted.begin(), fillings_sorted.end(), compare);

    //Print sorted fillings
    cout << "Sorted fillings:" << endl;
    for (auto i : fillings_sorted) {
        cout << i << endl;
    }

    //Calculate number of bowls there will be on the build screen
    //Get each ingredient in order, then get the number of times it appears from the map
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
    cout << "Number of bowls: " << bowls << endl;

    //We can finally start making the sandwich
    //Player must be on default sandwich menu
    enter_custom_sandwich_mode(env.program_info(), env.console, context);

    //Use the maps here when adding, as the maps already exist. Order does not matter when adding so we will use fillings_sorted when actually making the sandwich.
    add_sandwich_ingredients(env.realtime_dispatcher(), env.console, context, SANDWICH_OPTIONS.LANGUAGE, std::move(fillings), std::move(condiments));

    //calculate number of bowls
    //get filling from class
    //read the number of servings
    //determine bowls
    // servings/servings per bowl
    //then run loop for first bowl with coordinates from class?

    //for number of fillings
    //filling[i] coordinate sets?
    //use to prevent stacking all in one area
    //so set 1 coordinates would put them down the center
    //set 2 would try to go top, etc.

    //do a for loop?
    //need to define boxes/locations for fillings
    //also need to define how many fillings stack per bowl
    //ex. 1 serving of lettuce has 3 pieces in a single bowl
    // 2 servings of lettuce has 6 pieces in a single bowl
    // 3 servings of hamburger is 1 piece per bowl for a total of 3 bowls
    // 6 servings of cherry tomatoes is 9 pieces in two bowls
    
    wait_for_initial_hand(env.program_info(), env.console, context);
    env.log("Start making sandwich", COLOR_BLACK);

    const ImageFloatBox HAND_INITIAL_BOX{ 0.440, 0.455, 0.112, 0.179 };
    const ImageFloatBox center_bowl{ 0.455, 0.130, 0.090, 0.030 };
    const ImageFloatBox left_bowl{ 0.190, 0.136, 0.096, 0.031 };
    const ImageFloatBox right_bowl{ 0.715, 0.140, 0.108, 0.033 };
    const ImageFloatBox sandwich_target_box_left{ 0.386, 0.507, 0.060, 0.055 }; //where to place the ingredients
    const ImageFloatBox sandwich_target_box_middle{ 0.470, 0.507, 0.060, 0.055 };
    const ImageFloatBox sandwich_target_box_right{ 0.554, 0.507, 0.060, 0.055 };
    const ImageFloatBox upper_bread_drop_box{ 0.482, 0.400, 0.036, 0.030 };

    ImageFloatBox target_bowl = center_bowl;
    ImageFloatBox target_sandwich = sandwich_target_box_left;
    auto end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::FREE, false, HAND_INITIAL_BOX, target_bowl);
    context.wait_for_all_requests();
    
    for (auto i = 0; i < bowls; i++) {
        //Handle bowl locations
        switch (i) {
        case 0:
            //First ingredient is center - this has already been set
            break;
        case 1:
            target_bowl = left_bowl;
            break;
        case 2:
            target_bowl = right_bowl;
            break;
        case 3:
            //Press R
            pbf_press_button(context, BUTTON_R, 20, 80);
            target_bowl = left_bowl;
            break;
        case 4:
            //Press R
            pbf_press_button(context, BUTTON_R, 20, 80);
            target_bowl = left_bowl;
            break;
        default:
            break;
        }
        
        //Handle # of ingredients in the target bowl
        //Handle placement locations of the ingredient
        cout << "Num of ingredients in bowl: " << bowl_amounts.at(i) << endl;
        for (auto j = 0; j < bowl_amounts.at(i); j++) {

            end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_left);
            end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::FREE, false, { 0, 0, 1.0, 1.0 }, target_bowl);
            context.wait_for_all_requests();
        }
        end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_left);
        context.wait_for_all_requests();

    }






    //Most of the below is From make great peanut butter sandwich (for now)




    /*
    // console.overlay().add_log("Drop first banana", COLOR_WHITE);
    // visual feedback grabbing is not reliable. Switch to blind grabbing:
    


    // console.overlay().add_log("Pick second banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, { 0, 0, 1.0, 1.0 }, INGREDIENT_BOX);

    // console.overlay().add_log("Drop second banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_middle);

    // console.overlay().add_log("Pick third banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::FREE, false, { 0, 0, 1.0, 1.0 }, INGREDIENT_BOX);

    // console.overlay().add_log("Drop third banana", COLOR_WHITE);
    end_box = move_sandwich_hand(info, dispatcher, console, context, SandwichHandType::GRABBING, true, expand_box(end_box), sandwich_target_box_right);
    */

    /*
    // Drop upper bread and pick
    // console.overlay().add_log("Drop upper bread and pick", COLOR_WHITE);
    SandwichHandWatcher grabbing_hand(SandwichHandType::FREE, { 0, 0, 1.0, 1.0 });
    int ret = wait_until(env.console, context, std::chrono::seconds(30), { grabbing_hand });
    if (ret < 0) {
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, env.console,
            "SandwichMaker: Cannot detect grabing hand when waiting for upper bread.",
            grabbing_hand.last_snapshot()
        );
    }

    auto hand_box = hand_location_to_box(grabbing_hand.location());

    end_box = move_sandwich_hand(env.program_info(), env.realtime_dispatcher(), env.console, context, SandwichHandType::GRABBING, false, expand_box(hand_box), upper_bread_drop_box);
    pbf_mash_button(context, BUTTON_A, 125 * 5);

    env.log("Hand end box " + box_to_string(end_box));
    env.log("Built sandwich", COLOR_BLACK);
    */

    context.wait_for_all_requests();

    finish_sandwich_eating(env.program_info(), env.console, context);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

