/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

//#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch_Messages_Routines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game(VideoStream& stream, SwitchControllerContext& context){
    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen, 
    // regardless of whether the game is initially open or closed.

                                                     // if game initially open.  |  if game initially closed
    pbf_mash_button(context, BUTTON_X, 100);         // - Close game.            |  - does nothing
    pbf_press_dpad(context, DPAD_DOWN, 50, 50);      // - Does nothing.          |  - moves selector away from the closed game to avoid opening it.
    pbf_press_dpad(context, DPAD_DOWN, 50, 50);      // - Does nothing.          |  - Press Down a second time in case we drop one.
    pbf_mash_button(context, BUTTON_A, 50);          // - Confirm close game.    |  - opens an app on the home screen (e.g. Online)
    pbf_press_button(context, BUTTON_HOME, 50, 50);  // - Does nothing.          |  - goes back to home screen.
    context.wait_for_all_requests();

    // send a second Home button press, if the first one is dropped
    bool video_available = (bool)stream.video().snapshot();
    if (video_available){
        HomeWatcher detector;
        int ret = wait_until(
            stream, context,
            std::chrono::milliseconds(5000),
            { detector }
        );
        if (ret == 0){
            stream.log("Detected Home screen.");
        }else{                                               // if game initially open.  |  if game initially closed
            // initial Home button press was dropped
            pbf_press_button(context, BUTTON_HOME, 50, 50);  // - Does nothing.          |  - goes back to home screen, from opened app
        }
    }else{
        // - wait some time after first Home button press 
        // to avoid triggering zoom
        context.wait_for(std::chrono::milliseconds(1000));
        pbf_press_button(context, BUTTON_HOME, 50, 50);      // - Does nothing.          |  - Press Home a second time in case we drop one.
    }


    // fail-safe against button drops and unexpected error messages.
    pbf_mash_button(context, BUTTON_X, 50);
    pbf_mash_button(context, BUTTON_B, 350);
}



}
}
