/*  Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Functions to navigate main menu
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
// #include "CommonTools/VisualDetectors/BlackScreenDetector.h"
// #include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
// #include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
// #include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
// #include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
// #include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/PokemonLZA_MainMenuDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

void overworld_to_main_menu(ConsoleHandle& console, ProControllerContext& context){
    WallClock deadline = current_time() + std::chrono::seconds(120);
    while (current_time() < deadline){
        OverworldPartySelectionWatcher overworld(COLOR_RED, &console.overlay());
        MainMenuWatcher main_menu(COLOR_GREEN, &console.overlay());
        BoxWatcher box(COLOR_BLUE, &console.overlay());
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                box,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected Overworld...");
            pbf_press_button(context, BUTTON_X, 160ms, 240ms);
            continue;
        case 1:
            console.log("Detected Main Menu...");
            return;
        case 2:
            console.log("Detected Box System...");
            pbf_press_button(context, BUTTON_B, 160ms, 240ms);
            continue;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "overworld_to_main_menu(): No state detected after 30 seconds.",
                console
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "overworld_to_main_menu(): Failed to enter box system after 2 minutes.",
        console
    );
}

void overworld_to_box_system(ConsoleHandle& console, ProControllerContext& context){
    WallClock deadline = current_time() + std::chrono::seconds(120);
    while (current_time() < deadline){
        OverworldPartySelectionWatcher overworld(COLOR_RED, &console.overlay());
        MainMenuWatcher main_menu(COLOR_GREEN, &console.overlay());
        BoxWatcher box(COLOR_BLUE, &console.overlay());
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                overworld,
                main_menu,
                box,
            }
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            console.log("Detected Overworld...");
            pbf_press_button(context, BUTTON_X, 160ms, 240ms);
            continue;
        case 1:
            console.log("Detected Main Menu...");
            pbf_press_button(context, BUTTON_A, 160ms, 240ms);
            continue;
        case 2:
            console.log("Detected Box System...");
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "overworld_to_box_system(): No state detected after 30 seconds.",
                console
            );
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "overworld_to_box_system(): Failed to enter box system after 2 minutes.",
        console
    );
}


}
}
}
