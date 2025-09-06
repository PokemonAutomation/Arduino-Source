/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

//#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
//#include "NintendoSwitch_Messages_Routines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game_from_home(ConsoleHandle& console, ProControllerContext& context){
    ensure_at_home(console, context);

    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen, 
    // regardless of whether the game is initially open or closed.

                                                    // if game initially open.  |  if game initially closed
    pbf_mash_button(context, BUTTON_X, 100);        // - Close game.            |  - does nothing
    ssf_press_dpad_ptv(context, DPAD_DOWN);         // - Does nothing.          |  - moves selector away from the closed game to avoid opening it.
    ssf_press_dpad_ptv(context, DPAD_DOWN);         // - Does nothing.          |  - Press Down a second time in case we drop one.
    pbf_mash_button(context, BUTTON_A, 50);         // - Confirm close game.    |  - opens an app on the home screen (e.g. Online)
    go_home(console, context);                      // - Does nothing.          |  - goes back to home screen.

    // fail-safe against button drops and unexpected error messages.
    pbf_mash_button(context, BUTTON_X, 50);
    pbf_mash_button(context, BUTTON_B, 350);
}

void close_game_from_home(ConsoleHandle& console, JoyconContext& context){
    ensure_at_home(console, context);
    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen, 
    // regardless of whether the game is initially open or closed.

                                                        // if game initially open.  |  if game initially closed
    pbf_mash_button(context, BUTTON_X, 800ms);          // - Close game.            |  - does nothing
    pbf_move_joystick(context, 128, 255, 100ms, 10ms);  // - Does nothing.          |  - moves selector away from the closed game to avoid opening it.
    pbf_move_joystick(context, 128, 255, 100ms, 10ms);  // - Does nothing.          |  - Press Down a second time in case we drop one.
    pbf_mash_button(context, BUTTON_A, 400ms);          // - Confirm close game.    |  - opens an app on the home screen (e.g. Online)
    go_home(console, context);                          // - Does nothing.          |  - goes back to home screen.

    // fail-safe against button drops and unexpected error messages.
    pbf_mash_button(context, BUTTON_X, 400ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
}


}
}
