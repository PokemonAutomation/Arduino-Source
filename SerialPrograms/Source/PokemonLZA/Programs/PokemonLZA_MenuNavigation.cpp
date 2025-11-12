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
#include "PokemonLZA_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

void overworld_to_main_menu(ConsoleHandle& console, ProControllerContext& context){
    MainMenuWatcher main_menu_watcher(COLOR_RED, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for(int i = 0; i < 5; i++){
                pbf_press_button(context, BUTTON_X, 100ms, 3s);
            }
        },
        {{main_menu_watcher}}
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "overworld_to_main_menu(): No main menu detected after pressing \"X\" 5 times.",
            console
        );
    }
    console.log("Entered main menu.");
}

void overworld_to_box_system(ConsoleHandle& console, ProControllerContext& context){
    overworld_to_main_menu(console, context);
    BoxWatcher box_watcher(COLOR_RED, &console.overlay());
    int ret = run_until<ProControllerContext>(
        console, context,
        [&](ProControllerContext& context){
            for(int i = 0; i < 5; i++){
                pbf_press_button(context, BUTTON_A, 100ms, 2s);
            }
        },
        {{box_watcher}}
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "overworld_to_box_system(): No box system view detected after pressing \"A\" 5 times from main menu.",
            console
        );
    }
    console.log("Entered box system.");
}


}
}
}
