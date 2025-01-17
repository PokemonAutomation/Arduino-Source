/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/InterruptableCommands.tpp"
#include "CommonFramework/Tools/SuperControlSession.tpp"
#include "NintendoSwitch_Controller.h"

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::Controller>;
template class SuperControlSession<NintendoSwitch::Controller>;

namespace NintendoSwitch{






}
}
