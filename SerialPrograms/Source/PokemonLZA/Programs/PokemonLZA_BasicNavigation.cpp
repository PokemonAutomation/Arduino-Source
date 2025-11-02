/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
//#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
//#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA_BasicNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



void sit_on_bench(
    ConsoleHandle& console, ProControllerContext& context
){
#if 0
    while (true){
        ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0, 0, 1, 1}, &console.overlay());
        BlueDialogWatcher dialog(COLOR_YELLOW, &console.overlay());
        SelectionArrowWatcher arrow(COLOR_BLUE, &console.overlay(), SelectionArrowType::RIGHT, {0.5, 0.4, 0.3, 0.3});
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = wait_until(
            console, context,
            10000ms,
            {
                buttonA,
                dialog,
                arrow,
                black_screen,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            console.log("Detected floating A button...");
            pbf_press_button(context, BUTTON_A, 200ms, 200ms);
            continue;
        case 1:
            console.log("Detected dialog...");
            pbf_press_button(context, BUTTON_B, 200ms, 200ms);
            continue;
        case 2:
            console.log("Detected selection arrow...");
            pbf_press_button(context, BUTTON_A, 200ms, 200ms);
            continue;
        case 3:
            console.log("Detected day change.");
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "sit_on_bench(): No recognized state after 10 seconds.",
                console
            );
        }
    }
#endif

    {
        BlackScreenOverWatcher black_screen(COLOR_BLUE);

        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 5000ms);
                for (int c = 0; c < 3; c++){
                    pbf_move_left_joystick(context, 128, 255, 1000ms, 0ms);
                    pbf_mash_button(context, BUTTON_B, 1000ms);
                    pbf_mash_button(context, BUTTON_A, 5000ms);
                }
            },
            {black_screen}
        );

        switch (ret){
        case 0:
            console.log("Detected day change.");
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "sit_on_bench(): No transition detected after 4 attempts.",
                console
            );
        }
    }
    {
        ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.4, 0.3, 0.2, 0.7}, &console.overlay());

        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                //  Can't just hold it down since sometimes it doesn't register.
                for (int c = 0; c < 60; c++){
                    pbf_move_left_joystick(context, 128, 255, 800ms, 200ms);
                }
            },
            {buttonA}
        );

        switch (ret){
        case 0:
            console.log("Detected floating A button...");
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "sit_on_bench(): Unable to detect bench after 60 seconds.",
                console
            );
        }
    }

}



}
}
}
