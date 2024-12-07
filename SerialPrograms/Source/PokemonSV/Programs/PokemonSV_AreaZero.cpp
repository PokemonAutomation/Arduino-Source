/*  Area Zero
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_Navigation.h"
#include "PokemonSV_AreaZero.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void inside_zero_gate_to_station(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    int station,    //  1 - 4
    bool heal_at_station
){
    {
        AdvanceDialogWatcher dialog(COLOR_GREEN);
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 10 * TICKS_PER_SECOND, 0);
            },
            {dialog}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to find warp circle."
            );
        }
    }



    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to warp to station after 60 seconds."
            );
        }

        AdvanceDialogWatcher dialog(COLOR_GREEN);
        ZeroGateWarpPromptWatcher prompt(COLOR_GREEN);
        BlackScreenOverWatcher black_screen(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, std::chrono::seconds(5),
            {dialog, prompt, black_screen}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;
        case 1:
            console.log("Detected prompt.");
            prompt.move_cursor(info, console, context, station - 1);
            pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
            continue;
        case 2:
            console.log("Black screen is over. Arrive at station.");
            break;
        default:
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to find warp to station 2."
            );
        }

        break;
    }
    context.wait_for_all_requests();
    context.wait_for(std::chrono::seconds(3));

    if (heal_at_station){
        console.log("Moving to bed to heal.");
        pbf_move_left_joystick(context, 144, 0, 4 * TICKS_PER_SECOND, 0);
        ssf_press_left_joystick(context, 255, 128, 0, 125);
        bool healed = false;
        while (true){
            AdvanceDialogWatcher dialog(COLOR_GREEN);
            PromptDialogWatcher confirm(COLOR_GREEN);
            BlackScreenOverWatcher black_screen(COLOR_RED);
            OverworldWatcher overworld(console, COLOR_BLUE);
            context.wait_for_all_requests();
            int ret = wait_until(
                console, context, std::chrono::seconds(30),
                {dialog, confirm, black_screen, overworld}
            );
            context.wait_for(std::chrono::milliseconds(100));
            switch (ret){
            case 0:
                console.log("Detected dialog.");
                pbf_press_button(context, BUTTON_B, 20, 105);
                continue;
            case 1:
                console.log("Detected rest prompt.");
                pbf_press_button(context, BUTTON_A, 20, 105);
                continue;
            case 2:
                console.log("Done healing!");
                healed = true;
                continue;
            case 3:
                console.log("Detected overworld.");
                if (healed){
                    break;
                }else{
                    pbf_press_button(context, BUTTON_A, 20, 105);
                    continue;
                }
            default:
                OperationFailedException::fire(
                    console, ErrorReport::SEND_ERROR_REPORT,
                    "Heal at station: No state detected after 30 seconds."
                );
            }
            break;
        }
    }

    console.log("Exiting station. Waiting for black screen...");
    {
        BlackScreenOverWatcher black_screen(COLOR_RED);
        int ret = run_until(
            console, context,
            [=](BotBaseContext& context){
                if (heal_at_station){
                    pbf_move_left_joystick(context, 96, 255, 60 * TICKS_PER_SECOND, 0);
                }else{
                    pbf_move_left_joystick(context, 0, 255, 60 * TICKS_PER_SECOND, 0);
                }
            },
            {black_screen}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to exit station after 60 seconds."
            );
        }
    }

    console.log("Exiting station. Waiting for overworld...");
    {
        OverworldWatcher overworld(console, COLOR_BLUE);
        int ret = wait_until(
            console, context, std::chrono::seconds(30),
            {overworld}
        );
        if (ret < 0){
            OperationFailedException::fire(
                console, ErrorReport::SEND_ERROR_REPORT,
                "Unable to load overworld after exiting station for 30 seconds."
            );
        }
    }
}

void return_to_outside_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    open_map_from_overworld(info, console, context);
    pbf_move_left_joystick(context, 0, 0, 5, 50);
    fly_to_overworld_from_map(info, console, context);
}
void return_to_inside_zero_gate(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    return_to_outside_zero_gate(info, console, context);

    BlackScreenOverWatcher black_screen;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_move_left_joystick(context, 255, 32, 20, 105);
            pbf_mash_button(context, BUTTON_L, 60);
            pbf_move_left_joystick(context, 128, 0, 10 * TICKS_PER_SECOND, 0);
        },
        {black_screen}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Unable to enter Zero Gate."
        );
    }

    OverworldWatcher overworld(console);
    ret = wait_until(
        console, context, std::chrono::seconds(10),
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Unable to detect overworld inside Zero Gate."
        );
    }
}
void return_to_inside_zero_gate_from_picnic(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    BlackScreenOverWatcher black_screen;
    int ret = run_until(
        console, context,
        [](BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 255, 100, 40);
            pbf_mash_button(context, BUTTON_L, 60);
            pbf_move_left_joystick(context, 128, 0, 10 * TICKS_PER_SECOND, 0);
        },
        {black_screen}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Unable to enter Zero Gate."
        );
    }

    OverworldWatcher overworld(console);
    ret = wait_until(
        console, context, std::chrono::seconds(10),
        {overworld}
    );
    if (ret < 0){
        OperationFailedException::fire(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Unable to detect overworld inside Zero Gate."
        );
    }
}




void inside_zero_gate_to_secret_cave_entrance(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    bool heal_at_station
){
    inside_zero_gate_to_station(info, console, context, 1, heal_at_station);

    context.wait_for(std::chrono::seconds(3));

    pbf_move_left_joystick(context, 0, 208, 30, 50);
    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);


    //  Leg 1
    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_left_joystick(context, 128, 0, 60, 1250);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 20, 10); //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0, 20);     //  case one is dropped.

    pbf_move_left_joystick(context, 128, 0, 1125, 300);


    //  Leg 2
    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_left_joystick(context, 128, 0, 60, 250);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 20, 10); //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0, 20);     //  case one is dropped.

    pbf_move_left_joystick(context, 128, 0, 250, 0);
    pbf_move_left_joystick(context, 112, 0, 250, 0);
    pbf_move_left_joystick(context, 128, 0, 752, 0);
    pbf_move_left_joystick(context, 96, 0, 300, 0);
    pbf_move_left_joystick(context, 128, 0, 630, 250);

//    context.wait_for_all_requests();


    //  Leg 3
    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_left_joystick(context, 128, 0, 180, 550);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 20, 10); //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0, 20);     //  case one is dropped.

    pbf_move_left_joystick(context, 48, 0, 200, 0);
    pbf_move_left_joystick(context, 128, 0, 250, 0);


}




}
}
}
