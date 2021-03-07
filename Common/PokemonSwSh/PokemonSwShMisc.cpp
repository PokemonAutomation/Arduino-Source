/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"

using namespace PokemonAutomation;

void mash_A(uint16_t ticks){
    mash_A(*global_connection, ticks);
}
void mash_A(BotBase& device, uint16_t ticks){
    pabb_mashA params;
    params.ticks = ticks;
    device.issue_request<PABB_MSG_COMMAND_MASH_A>(params);
}

void IoA_backout(uint16_t pokemon_to_menu_delay){
    IoA_backout(*global_connection, pokemon_to_menu_delay);
}
void IoA_backout(BotBase& device, uint16_t pokemon_to_menu_delay){
    pabb_IoA_backout params;
    params.pokemon_to_menu_delay = pokemon_to_menu_delay;
    device.issue_request<PABB_MSG_COMMAND_IOA_BACKOUT>(params);
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
