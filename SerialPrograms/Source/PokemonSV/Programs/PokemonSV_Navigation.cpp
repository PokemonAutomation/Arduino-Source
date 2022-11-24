/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void save_game_from_menu(ConsoleHandle& console, BotBaseContext& context){
    {
        GradientArrowFinder detector(console.overlay(), GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_R, 20, 5 * TICKS_PER_SECOND);
            },
            {detector}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "Unable to find save confirmation prompt.");
        }
        console.log("Detected save confirmation prompt.");
    }
    pbf_press_button(context, BUTTON_A, 20, 2 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    {
        AdvanceDialogFinder detector;
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {detector}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "Unable to find save finished dialog.");
        }
        console.log("Detected save finished dialog.");
    }
    {
        MainMenuFinder detector;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_B, 20, 10 * TICKS_PER_SECOND);
            },
            {detector}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "Unable to detect main menu after 10 seconds.");
        }
        console.log("Returned to main menu.");
    }
}
void save_game_from_overworld(ConsoleHandle& console, BotBaseContext& context){
    {
        MainMenuFinder detector;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_X, 20, 10 * TICKS_PER_SECOND);
            },
            {detector}
        );
        if (ret < 0){
            throw OperationFailedException(console.logger(), "Unable to detect main menu after 10 seconds.");
        }
        console.log("Detected to main menu.");
    }
    context.wait_for_all_requests();
    save_game_from_menu(console, context);
    pbf_press_button(context, BUTTON_B, 20, 230);
}



bool open_raid(ConsoleHandle& console, BotBaseContext& context){
    TeraCardFinder card_detector;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 355);
        },
        {card_detector}
    );
    if (ret < 0){
        console.log("No Tera raid found.", COLOR_ORANGE);
        return false;
    }

    console.log("Tera raid found!", COLOR_BLUE);
    return true;
}



}
}
}
