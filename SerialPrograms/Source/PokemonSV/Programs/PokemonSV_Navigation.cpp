/*  Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/PokemonSV_Settings.h"
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
    bool saved = false;
    while (true){
        context.wait_for_all_requests();

        MainMenuWatcher menu(COLOR_RED);
        GradientArrowWatcher confirmation(COLOR_YELLOW, console.overlay(), GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
        AdvanceDialogWatcher finished(COLOR_GREEN);

        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {menu, confirmation, finished}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            if (saved){
                console.log("Detected main menu. Finished!");
                return;
            }else{
                console.log("Detected main menu. Saving game...");
                pbf_press_button(context, BUTTON_R, 20, 105);
                continue;
            }
        case 1:
            console.log("Detected save confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 2:
            console.log("Detected save finished dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            saved = true;
            continue;
        default:
            throw OperationFailedException(console.logger(), "save_game_from_menu(): No recognized state after 60 seconds.");
        }
    }
}
void save_game_from_overworld(ConsoleHandle& console, BotBaseContext& context){
    {
        MainMenuWatcher detector;
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
    TeraCardWatcher card_detector(COLOR_RED);
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
        MainMenuWatcher detector;
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
        MainMenuWatcher main_menu(COLOR_RED);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
        PromptDialogWatcher prompt(COLOR_CYAN);
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

void set_time_to_1am_from_home(ConsoleHandle& console, BotBaseContext& context){
    DateReader reader;
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, false);
    pbf_press_button(context, BUTTON_A, 20, 105);
    reader.set_hours(console, context, 1);
    pbf_press_button(context, BUTTON_A, 20, 105);
    pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
//    resume_game_from_home(console, context);
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

        TeraCatchWatcher catch_menu(COLOR_BLUE);
        WhiteButtonWatcher next_button(
            COLOR_RED,
            WhiteButton::ButtonA, 20,
            console.overlay(),
            {0.8, 0.93, 0.2, 0.07}
        );
        BlackScreenOverWatcher black_screen(COLOR_MAGENTA);
        WhiteScreenOverWatcher white_screen(COLOR_MAGENTA);
        AdvanceDialogWatcher dialog(COLOR_YELLOW);
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
