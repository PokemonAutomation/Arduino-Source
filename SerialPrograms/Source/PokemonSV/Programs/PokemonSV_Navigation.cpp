/*  Navigation
 exit_tera_without_catching/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
//#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
//#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void save_game_from_menu(ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool saved = false;
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to save game after 5 minutes.");
        }

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
    WallClock start = current_time();
    bool saved = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to save game after 5 minutes.");
        }

        int ret;
        {
            OverworldWatcher overworld(COLOR_CYAN);
            MainMenuWatcher main_menu(COLOR_RED);
            context.wait_for_all_requests();
            ret = wait_until(
                console, context,
                std::chrono::seconds(60),
                {overworld, main_menu}
            );
        }
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            if (saved){
                return;
            }else{
                pbf_press_button(context, BUTTON_X, 20, 105);
                continue;
            }
        case 1:
            console.log("Detected main menu.");
            if (!saved){
                save_game_from_menu(console, context);
                saved = true;
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            throw OperationFailedException(console.logger(), "save_game_from_overworld(): No recognized state after 60 seconds.");
        }
    }
}



void connect_to_internet_from_overworld(ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(console, "Failed to connect to internet after 5 minutes.");
        }

        OverworldWatcher overworld(COLOR_RED);
        MainMenuWatcher main_menu(COLOR_YELLOW);
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        PromptDialogWatcher prompt(COLOR_CYAN);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, main_menu, dialog, prompt}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            if (connected){
                return;
            }else{
                pbf_press_button(context, BUTTON_X, 20, 105);
                continue;
            }
        case 1:
            console.log("Detected main menu.");
            if (connected){
                pbf_press_button(context, BUTTON_B, 20, 105);
            }else{
                pbf_press_button(context, BUTTON_L, 20, 105);
            }
            continue;
        case 2:
            console.log("Detected dialog.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 3:
            console.log("Already connected to internet.");
            connected = true;
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            throw OperationFailedException(console.logger(), "connect_to_internet_from_overworld(): No recognized state after 60 seconds.");
        }
    }
}

void set_time_to_12am_from_home(ConsoleHandle& console, BotBaseContext& context){
    DateReader reader;
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, false);
    pbf_press_button(context, BUTTON_A, 20, 105);
    reader.set_hours(console, context, 0);
    pbf_press_button(context, BUTTON_A, 20, 105);
    pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
//    resume_game_from_home(console, context);
}







}
}
}
