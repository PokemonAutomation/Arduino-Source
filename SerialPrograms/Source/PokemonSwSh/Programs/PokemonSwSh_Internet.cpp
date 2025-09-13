/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh_Internet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet_with_inference(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    Milliseconds post_wait_time,
    Milliseconds timeout_ticks
){
    stream.log("Connecting to internet...");
    //  Enter Y-COMM.
    bool ok = true;
//    cout << "Waiting for Y-COMM to open..." << endl;
    for (size_t attempts = 0;;){
        YCommMenuDetector detector(true);
        if (detector.detect(stream.video().snapshot())){
            stream.log("Y-COMM detected.");
            break;
        }

        int result = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_Y, 200ms, 4800ms);
            },
            {detector}
        );
        if (result == 0){
            stream.log("Y-COMM detected.");
            break;
        }
        stream.log("Failed to detect Y-COMM.", COLOR_RED);

        attempts++;
        if (attempts >= 9){
            dump_image(
                stream.logger(), info,
                "connect_to_internet_with_inference",
                stream.video().snapshot()
            );
            ok = false;
            break;
        }
    }

    //  Connect to internet.
    context.wait_for(std::chrono::seconds(1));
    pbf_press_dpad(context, DPAD_UP, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_mash_button(context, BUTTON_PLUS, TICKS_PER_SECOND);
    context.wait_for_all_requests();

//    cout << "Waiting for Y-COMM to close..." << endl;
    //  Mash B until you leave Y-COMM.
    {
        YCommMenuDetector detector(false);
        int result = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, timeout_ticks);
            },
            {{detector}}
        );
        if (result == 0){
            stream.log("Y-COMM no longer detected. Assume done connected to internet.");
        }else{
            stream.log("Failed to see Y-COMM go away after timeout.", COLOR_RED);
            ok = false;
        }
    }

    //  Extra wait.
    context.wait_for(post_wait_time);

    return ok;
}


}
}
}
