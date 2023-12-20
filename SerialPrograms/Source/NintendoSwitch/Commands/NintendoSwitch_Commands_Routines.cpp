/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_Commands_Routines.h"
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
    ssf_mash1_button(context, BUTTON_X, 100);           //  Close game
    ssf_mash2_button(context, BUTTON_X, BUTTON_A, 50);  //  Confirm close game
    ssf_mash1_button(context, BUTTON_X, 50);
    ssf_mash1_button(context, BUTTON_B, 350);
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
