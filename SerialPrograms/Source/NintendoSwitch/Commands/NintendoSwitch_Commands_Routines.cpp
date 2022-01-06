/*  General Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch_Messages_Routines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void close_game(const BotBaseContext& context){
    context.issue_request(
        DeviceRequest_close_game()
    );
}



int register_message_converters_routines(){
    register_message_converter(
        PABB_MSG_COMMAND_CLOSE_GAME,
        [](const std::string& body){
            std::stringstream ss;
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


}
}
