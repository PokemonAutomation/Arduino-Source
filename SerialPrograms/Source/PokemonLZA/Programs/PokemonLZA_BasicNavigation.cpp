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
#include "PokemonLZA/Inference/PokemonLZA_MapDetector.h"
#include "PokemonLZA_BasicNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


void ensure_map(ConsoleHandle& console, ProControllerContext& context){
    WallClock deadline = current_time() + 30000ms;
    do{
        context.wait_for_all_requests();

        MapWatcher map_detector(COLOR_RED, &console.overlay());

        int ret = wait_until(
            console, context,
            3000ms,
            {map_detector}
        );
        switch (ret){
        case 0:
            console.log("Detected map!", COLOR_BLUE);
            return;
        default:
            console.log("Unable to detect map.", COLOR_ORANGE);
            pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms);
        }

    }while (current_time() < deadline);

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "open_map(): Unable to find map after 30 seconds.",
        console
    );
}
void open_map(ConsoleHandle& console, ProControllerContext& context){
    console.log("Opening Map...");
    pbf_press_button(context, BUTTON_PLUS, 240ms, 80ms);
    ensure_map(console, context);
}
bool fly_from_map(ConsoleHandle& console, ProControllerContext& context){
    console.log("Flying from map...");

    {
        BlackScreenWatcher start_flying(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 5000ms);
            },
            {start_flying,}
        );
        switch (ret){
        case 0:
            console.log("Flying from map... Started!");
            break;
        default:
            return false;
#if 0
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_map(): Unable to fly.",
                console
            );
#endif
        }
    }
    {
        BlackScreenOverWatcher done_flying(COLOR_RED, {0.1, 0.7, 0.8, 0.2});
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 30000ms);
            },
            {done_flying,}
        );
        switch (ret){
        case 0:
            console.log("Flying from map... Done!");
            break;
        default:
            return false;
#if 0
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fly_from_map(): Unable to fly. Timed out.",
                console
            );
#endif
        }
    }

    return true;
}




void sit_on_bench(ConsoleHandle& console, ProControllerContext& context){
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
