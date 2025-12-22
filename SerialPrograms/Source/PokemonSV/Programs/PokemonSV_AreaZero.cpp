/*  Area Zero
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_AreaZero.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void inside_zero_gate_to_station(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    int station,    //  1 - 4
    bool heal_at_station
){
    {
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, {0, +1}, 10000ms, 0ms);
            },
            {dialog}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to find warp circle.",
                stream
            );
        }
    }



    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to warp to station after 60 seconds.",
                stream
            );
        }

        AdvanceDialogWatcher dialog(COLOR_GREEN);
        ZeroGateWarpPromptWatcher prompt(COLOR_GREEN);
        BlackScreenOverWatcher black_screen(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(5),
            {dialog, prompt, black_screen}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            stream.log("Detected dialog.");
            pbf_press_button(context, BUTTON_A, 160ms, 240ms);
            continue;
        case 1:
            stream.log("Detected prompt.");
            prompt.move_cursor(info, stream, context, station - 1);
            pbf_mash_button(context, BUTTON_A, 3000ms);
            continue;
        case 2:
            stream.log("Black screen is over. Arrive at station.");
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to find warp to station 2.",
                stream
            );
        }

        break;
    }
    context.wait_for_all_requests();
    context.wait_for(std::chrono::seconds(3));

    if (heal_at_station){
        stream.log("Moving to bed to heal.");
        pbf_move_left_joystick(context, 144, 0, 4000ms, 0ms);
        ssf_press_left_joystick(context, {+1, 0}, 0ms, 1000ms);
        bool healed = false;
        while (true){
            AdvanceDialogWatcher dialog(COLOR_GREEN);
            PromptDialogWatcher confirm(COLOR_GREEN);
            BlackScreenOverWatcher black_screen(COLOR_RED);
            OverworldWatcher overworld(stream.logger(), COLOR_BLUE);
            context.wait_for_all_requests();
            int ret = wait_until(
                stream, context, std::chrono::seconds(30),
                {dialog, confirm, black_screen, overworld}
            );
            context.wait_for(std::chrono::milliseconds(100));
            switch (ret){
            case 0:
                stream.log("Detected dialog.");
                pbf_press_button(context, BUTTON_B, 160ms, 840ms);
                continue;
            case 1:
                stream.log("Detected rest prompt.");
                pbf_press_button(context, BUTTON_A, 160ms, 840ms);
                continue;
            case 2:
                stream.log("Done healing!");
                healed = true;
                continue;
            case 3:
                stream.log("Detected overworld.");
                if (healed){
                    break;
                }else{
                    pbf_press_button(context, BUTTON_A, 160ms, 840ms);
                    continue;
                }
            default:
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Heal at station: No state detected after 30 seconds.",
                    stream
                );
            }
            break;
        }
    }

    stream.log("Exiting station. Waiting for black screen...");
    {
        BlackScreenOverWatcher black_screen(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [=](ProControllerContext& context){
                if (heal_at_station){
                    pbf_move_left_joystick(context, 96, 255, 60000ms, 0ms);
                }else{
                    pbf_move_left_joystick(context, {-1, -1}, 60000ms, 0ms);
                }
            },
            {black_screen}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to exit station after 60 seconds.",
                stream
            );
        }
    }

    stream.log("Exiting station. Waiting for overworld...");
    {
        OverworldWatcher overworld(stream.logger(), COLOR_BLUE);
        int ret = wait_until(
            stream, context, std::chrono::seconds(30),
            {overworld}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to load overworld after exiting station for 30 seconds.",
                stream
            );
        }
    }
}

void return_to_outside_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    open_map_from_overworld(info, stream, context);
    pbf_move_left_joystick(context, 96, 96, 5, 50);
    fly_to_overworld_from_map(info, stream, context);
}
void return_to_inside_zero_gate(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    return_to_outside_zero_gate(info, stream, context);

    BlackScreenOverWatcher black_screen;
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, 255, 32, 160ms, 840ms);
            pbf_mash_button(context, BUTTON_L, 60);
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 0ms);
        },
        {black_screen}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to enter Zero Gate.",
            stream
        );
    }

    OverworldWatcher overworld(stream.logger());
    ret = wait_until(
        stream, context, std::chrono::seconds(10),
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to detect overworld inside Zero Gate.",
            stream
        );
    }
}
void return_to_inside_zero_gate_from_picnic(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    BlackScreenOverWatcher black_screen;
    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, {0, -1}, 800ms, 320ms);
            pbf_mash_button(context, BUTTON_L, 60);
            pbf_move_left_joystick(context, {0, +1}, 10000ms, 0ms);
        },
        {black_screen}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to enter Zero Gate.",
            stream
        );
    }

    OverworldWatcher overworld(stream.logger());
    ret = wait_until(
        stream, context, std::chrono::seconds(10),
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to detect overworld inside Zero Gate.",
            stream
        );
    }
}




void inside_zero_gate_to_secret_cave_entrance(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    bool heal_at_station
){
    inside_zero_gate_to_station(info, stream, context, 1, heal_at_station);

    context.wait_for(std::chrono::seconds(3));

    pbf_move_left_joystick(context, 0, 208, 30, 50);
    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 160ms, 840ms);


    //  Leg 1
    ssf_press_button(context, BUTTON_LCLICK, 0ms, 4000ms);
    ssf_press_left_joystick(context, {0, +1}, 480ms, 10000ms);

    //  Jump
    ssf_press_button(context, BUTTON_B, 1000ms, 800ms);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0ms, 160ms, 80ms);  //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);        //  case one is dropped.

    pbf_move_left_joystick(context, {0, +1}, 9000ms, 2400ms);


    //  Leg 2
    ssf_press_button(context, BUTTON_LCLICK, 0ms, 4000ms);
    ssf_press_left_joystick(context, {0, +1}, 480ms, 2000ms);

    //  Jump
    ssf_press_button(context, BUTTON_B, 1000ms, 800ms);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0ms, 160ms, 80ms);  //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);        //  case one is dropped.

    pbf_move_left_joystick(context, {0, +1}, 2000ms, 0ms);
    pbf_move_left_joystick(context, 112, 0, 2000ms, 0ms);
    pbf_move_left_joystick(context, {0, +1}, 6016ms, 0ms);
    pbf_move_left_joystick(context, 96, 0, 2400ms, 0ms);
    pbf_move_left_joystick(context, {0, +1}, 5040ms, 2000ms);

//    context.wait_for_all_requests();


    //  Leg 3
    ssf_press_button(context, BUTTON_LCLICK, 0ms, 4000ms);
    ssf_press_left_joystick(context, {0, +1}, 1440ms, 4400ms);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0ms, 160ms, 80ms);  //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);        //  case one is dropped.

    pbf_move_left_joystick(context, 48, 0, 1600ms, 0ms);
    pbf_move_left_joystick(context, {0, +1}, 2000ms, 0ms);


}




}
}
}
