/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV_ConnectToInternet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void connect_to_internet_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "ConnectInternetFailed",
                "connect_to_internet_from_menu(): Failed to connect to internet after 5 minutes."
            );
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
            console.log("Detected overworld. (unexpected)", COLOR_RED);
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            console.log("Detected main menu.");
            if (connected){
                return;
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
            dump_image_and_throw_recoverable_exception(
                info, console, "ConnectInternetFailed",
                "connect_to_internet_from_menu(): No recognized state after 60 seconds."
            );
        }
    }
}
void connect_to_internet_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    WallClock start = current_time();
    bool connected = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "ConnectInternetFailed",
                "connect_to_internet_from_overworld(): Failed to connect to internet after 5 minutes."
            );
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
            dump_image_and_throw_recoverable_exception(
                info, console, "ConnectInternetFailed",
                "connect_to_internet_from_overworld(): No recognized state after 60 seconds."
            );
        }
    }
}




}
}
}
