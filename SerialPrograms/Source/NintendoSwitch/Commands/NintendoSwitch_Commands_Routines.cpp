/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch_Messages_Routines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game(ConsoleHandle& console, BotBaseContext& context){
#if 0
    context.issue_request(
        DeviceRequest_close_game()
    );
#else
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
    bool video_available = (bool)console.video().snapshot();
    if (video_available){
        HomeWatcher detector;
        int ret = wait_until(
            console, context,
            std::chrono::milliseconds(5000),
            { detector }
        );
        if (ret == 0){
            console.log("Detected Home screen.");
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

#endif
}


#if 0
int register_message_converters_routines(){
    register_message_converter(
        PABB_MSG_COMMAND_CLOSE_GAME,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "close_game() - ";
            if (body.size() != sizeof(pabb_close_game)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_close_game*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    return 0;
}
int init_NintendoSwitchRoutines = register_message_converters_routines();
#endif


}
}
