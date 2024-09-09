/*  Save Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_SaveGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{





void save_game_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    save_game_from_menu_or_overworld(info, console, context, false);
}

void save_game_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    save_game_from_menu_or_overworld(info, console, context, true);
}



void save_game_from_menu_or_overworld(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    bool return_to_overworld
){
    context.wait_for_all_requests();
    console.log("Saving game...");
    console.overlay().add_log("Save game", COLOR_YELLOW);
    WallClock start = current_time();
    bool saved = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(5)){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "save_game_from_menu_or_overworld(): Failed to save game after 5 minutes.",
                true
            );
        }

        OverworldWatcher overworld(COLOR_CYAN);
        MainMenuWatcher menu(COLOR_RED);
        GradientArrowWatcher confirmation(COLOR_YELLOW, GradientArrowType::RIGHT, {0.72, 0.55, 0.05, 0.08});
        AdvanceDialogWatcher finished(COLOR_GREEN);
        context.wait_for_all_requests();

        int ret = wait_until(
            console, context,
            std::chrono::seconds(60),
            {overworld, menu, confirmation, finished}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected overworld.");
            if (saved && return_to_overworld){
                return;
            }
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            if (!saved){
                console.log("Detected main menu. Saving game...");
                pbf_press_button(context, BUTTON_R, 20, 105);
                continue;
            }
            if (return_to_overworld){
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            }
            return;
        case 2:
            console.log("Detected save confirmation prompt.");
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3:
            console.log("Detected save finished dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            saved = true;
            continue;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "save_game_from_menu_or_overworld(): No recognized state after 60 seconds.",
                true
            );
        }





    }

}


void save_game_tutorial(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    
    // open the menu.
    MainMenuWatcher menu(COLOR_RED);
    int ret0 = run_until(
        console, context,
        [](BotBaseContext& context){
            for (size_t i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_X, 20, 500);
            }
        },
        {menu}
    );
    if (ret0 != 0){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, console,
            "Failed to open menu!",
            true
        );
    }    

    save_game_from_menu(info, console, context);
    pbf_mash_button(context, BUTTON_B, 200);

}




}
}
}
