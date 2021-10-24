/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwSh_Commands_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void mash_A(const BotBaseContext& context, uint16_t ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_mash_A(ticks)
    );
}
void IoA_backout(const BotBaseContext& context, uint16_t pokemon_to_menu_delay){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_IoA_backout(pokemon_to_menu_delay)
    );
}

int register_message_converters_pokemon_misc(){
    register_message_converter(
        PABB_MSG_COMMAND_MASH_A,
        [](const std::string& body){
            std::stringstream ss;
            ss << "mash_A() - ";
            if (body.size() != sizeof(pabb_mashA)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_mashA*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_IOA_BACKOUT,
        [](const std::string& body){
            std::stringstream ss;
            ss << "IoA_backout() - ";
            if (body.size() != sizeof(pabb_IoA_backout)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_IoA_backout*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", pokemon_to_menu_delay = " << params->pokemon_to_menu_delay;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_CONTROLLER_STATE,
        [](const std::string& body){
            std::stringstream ss;
            ss << "controller_state() - ";
            if (body.size() != sizeof(pabb_controller_state)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_controller_state*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button;
            ss << ", dpad = " << (int)params->dpad;
            ss << ", LJ = (" << (int)params->left_joystick_x << "," << (int)params->left_joystick_y << ")";
            ss << ", RJ = (" << (int)params->right_joystick_x << "," << (int)params->right_joystick_y << ")";
            ss << ", ticks = " << (int)params->ticks;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShMisc = register_message_converters_pokemon_misc();



}
}


