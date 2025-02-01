/*  Nintendo Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "NintendoSwitch_Controller.h"

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<NintendoSwitch::SwitchController>;
template class SuperControlSession<NintendoSwitch::SwitchController>;

namespace NintendoSwitch{










}
}
