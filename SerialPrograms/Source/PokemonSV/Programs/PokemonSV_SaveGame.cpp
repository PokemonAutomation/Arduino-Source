/*  Save Game
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
#include "PokemonSV_SaveGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void save_game_from_menu_impl(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Saving game from menu...");
    WallClock start = current_time();
    bool saved = false;
    while (true){
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "SaveGameFailed",
                "save_game_from_menu(): Failed to save game after 5 minutes."
            );
        }

        MainMenuWatcher menu(COLOR_RED);
        GradientArrowWatcher confirmation(COLOR_YELLOW, GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
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
            dump_image_and_throw_recoverable_exception(
                info, console, "SaveGameFailed",
                "save_game_from_menu(): No recognized state after 60 seconds."
            );
        }
    }
}



void save_game_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.overlay().add_log("Save game", COLOR_YELLOW);
    save_game_from_menu_impl(info, console, context);
}

void save_game_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.overlay().add_log("Save game", COLOR_YELLOW);
    console.log("Saving game from overworld...");
    WallClock start = current_time();
    bool saved = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            dump_image_and_throw_recoverable_exception(
                info, console, "SaveGameFailed",
                "save_game_from_overworld(): Failed to save game after 5 minutes."
            );
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
                save_game_from_menu_impl(info, console, context);
                saved = true;
            }
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            dump_image_and_throw_recoverable_exception(
                info, console, "SaveGameFailed",
                "save_game_from_overworld(): No recognized state after 60 seconds."
            );
        }
    }
}






}
}
}
