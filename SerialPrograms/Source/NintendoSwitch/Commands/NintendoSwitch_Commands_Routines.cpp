/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch_Messages_Routines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game(BotBaseContext& context){
#if 0
    context.issue_request(
        DeviceRequest_close_game()
    );
#else
    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen, 
    // regardless of whether the game is open or closed.
    pbf_mash_button(context, BUTTON_X, 100);    // if game open: Close game. if game closed: does nothing
    pbf_press_dpad(context, DPAD_DOWN, 50, 50); // if game open: Does nothing. if game closed: moves selector away from the closed game to avoid opening it.
    pbf_press_dpad(context, DPAD_DOWN, 50, 50); // extra insurance against button drops.
    pbf_mash_button(context, BUTTON_A, 50);     // if game open: Confirm close game. if game closed: opens an app in the home screen (e.g. Online)
    pbf_mash_button(context, BUTTON_HOME, 50);  // if game open: Does nothing. if game closed: closes the app and goes back to home screen.
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
