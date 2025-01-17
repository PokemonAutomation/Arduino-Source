/*  Misc. Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_Misc.h"
//#include "PokemonSwSh_Messages_Misc.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


#if 0
void mash_A(ControllerContext& context, uint16_t ticks){
    context.issue_request(
        DeviceRequest_mash_A(ticks)
    );
}
#endif
void IoA_backout(ControllerContext& context, uint16_t pokemon_to_menu_delay){
#if 0
    context.issue_request(
        DeviceRequest_IoA_backout(pokemon_to_menu_delay)
    );
#else
    //  Back out to menu.
    //  Prepend each B press by a DOWN press so that the B gets
    //  swallowed while in the summary.
    ssf_press_dpad(context, DPAD_DOWN, 5, 15);
    ssf_press_button(context, BUTTON_B, 60, 10);
    ssf_press_dpad(context, DPAD_DOWN, 5, 15);
    ssf_press_button(context, BUTTON_B, pokemon_to_menu_delay, 10);
#endif
}


#if 0
int register_message_converters_pokemon_misc(){
    register_message_converter(
        PABB_MSG_COMMAND_MASH_A,
        [](const std::string& body){
            std::ostringstream ss;
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
            std::ostringstream ss;
            ss << "IoA_backout() - ";
            if (body.size() != sizeof(pabb_IoA_backout)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_IoA_backout*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", pokemon_to_menu_delay = " << params->pokemon_to_menu_delay;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShMisc = register_message_converters_pokemon_misc();
#endif


}
}


