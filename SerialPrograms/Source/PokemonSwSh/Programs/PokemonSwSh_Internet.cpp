/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh_Internet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet_with_inference(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    std::chrono::milliseconds post_wait_time,
    uint16_t timeout_ticks
){
    console.log("Connecting to internet...");
    //  Enter Y-COMM.
    bool ok = true;
//    cout << "Waiting for Y-COMM to open..." << endl;
    {
        YCommMenuDetector detector(true);
        if (!detector.detect(console.video().snapshot())){
            pbf_press_button(console, BUTTON_Y, 10, TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();
        }
        int result = wait_until(
            env, context, console,
            std::chrono::seconds(10),
            { &detector }
        );
        if (result == 0){
            console.log("Y-COMM detected.");
        }else{
            console.log("Failed to detect Y-COMM after timeout.", COLOR_RED);
            dump_image(console, ProgramInfo(), "connect_to_internet_with_inference", console.video().snapshot());
            ok = false;
        }
    }

    //  Connect to internet.
    env.wait_for(std::chrono::seconds(1));
    pbf_press_dpad(console, DPAD_UP, 5, 0);
    pbf_move_right_joystick(console, 128, 0, 5, 0);
    pbf_mash_button(console, BUTTON_PLUS, TICKS_PER_SECOND);
    console.botbase().wait_for_all_requests();

//    cout << "Waiting for Y-COMM to close..." << endl;
    //  Mash B until you leave Y-COMM.
    {
        YCommMenuDetector detector(false);
        int result = run_until(
            env, context, console,
            [&](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, timeout_ticks);
            },
            { &detector }
        );
        if (result == 0){
            console.log("Y-COMM no longer detected. Assume done connected to internet.");
        }else{
            console.log("Failed to see Y-COMM go away after timeout.", COLOR_RED);
            ok = false;
        }
    }

    //  Extra wait.
    env.wait_for(post_wait_time);

    return ok;
}


}
}
}
