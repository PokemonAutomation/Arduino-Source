/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void save_game_from_menu(ConsoleHandle& console, BotBaseContext& context){
    {
        GradientArrowFinder detector(COLOR_RED, console.overlay(), GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
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
    context.wait_for(std::chrono::milliseconds(100));
    pbf_press_button(context, BUTTON_A, 20, 2 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    {
        AdvanceDialogFinder detector(COLOR_RED);
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
    context.wait_for(std::chrono::milliseconds(100));
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
    context.wait_for(std::chrono::milliseconds(100));
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
    context.wait_for(std::chrono::milliseconds(100));
//    context.wait_for_all_requests();
    save_game_from_menu(console, context);
    pbf_press_button(context, BUTTON_B, 20, 230);
}



bool open_raid(ConsoleHandle& console, BotBaseContext& context){
    TeraCardFinder card_detector(COLOR_RED);
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

void connect_to_internet_from_overworld(ConsoleHandle& console, BotBaseContext& context){
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
    context.wait_for(std::chrono::milliseconds(100));

    console.log("Connecting to the internet...");
    pbf_press_button(context, BUTTON_L, 20, 230);
    context.wait_for_all_requests();
    while (true){
        MainMenuFinder main_menu(COLOR_RED);
        AdvanceDialogFinder dialog(COLOR_YELLOW);
        PromptDialogFinder prompt(COLOR_CYAN);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {main_menu, dialog, prompt}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Finished connecting to the internet.");
            pbf_press_button(context, BUTTON_B, 20, 230);
            context.wait_for_all_requests();
            return;
        case 1:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        case 2:
            console.log("Already connected to internet.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            break;
        default:
            throw OperationFailedException(console.logger(), "Unable to connect to internet after 60 seconds.");
        }
    }
}


void exit_tera_win_without_catching(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    BotBaseContext& context
){
    //  State machine to return to overworld.
    bool black_screen_detected = false;
    bool white_screen_detected = false;
    std::chrono::seconds timeout = std::chrono::seconds(60);
    while (true){
        context.wait_for_all_requests();
        env.log("Looking for post raid dialogs...");

        TeraCatchFinder catch_menu(COLOR_BLUE);
        WhiteButtonFinder next_button(
            COLOR_RED,
            WhiteButton::ButtonA, 20,
            console.overlay(),
            {0.8, 0.93, 0.2, 0.07}
        );
        BlackScreenOverWatcher black_screen(COLOR_MAGENTA);
        WhiteScreenOverWatcher white_screen(COLOR_MAGENTA);
        AdvanceDialogFinder dialog(COLOR_YELLOW);
        int ret = wait_until(
            console, context,
            timeout,
            {
                catch_menu,
                next_button,
                black_screen,
                white_screen,
                dialog,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            env.log("Detected catch prompt.");
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_mash_button(context, BUTTON_A, 50);
            timeout = std::chrono::seconds(60);
            break;
        case 1:
            env.log("Detected possible (A) Next button.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 105);
            timeout = std::chrono::seconds(60);
            break;
        case 2:
            env.log("Detected black screen is over.");
            black_screen_detected = true;
            timeout = std::chrono::seconds(60);
            context.wait_for(std::chrono::seconds(1));
            break;
        case 3:
            env.log("Detected white screen is over.");
            //  If both white and black screens are over, we are out of the raid.
            if (white_screen_detected && black_screen_detected){
                timeout = std::chrono::seconds(4);
            }else{
                timeout = std::chrono::seconds(60);
            }
            white_screen_detected = true;
            context.wait_for(std::chrono::seconds(1));
            break;
        case 4:
            env.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            timeout = std::chrono::seconds(5);
            break;
        default:
            env.log("No detection, assume returned to overworld.");
            return;
        }
    }
}






}
}
}
