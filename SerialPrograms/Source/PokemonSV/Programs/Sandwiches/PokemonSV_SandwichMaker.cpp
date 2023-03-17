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
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV_SandwichMaker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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

    //Player must be on default sandwich menu
    enter_custom_sandwich_mode(env.program_info(), env.console, context);
    add_sandwich_ingredients(env.realtime_dispatcher(), env.console, context, SANDWICH_OPTIONS.LANGUAGE, std::move(fillings), std::move(condiments));


    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}

