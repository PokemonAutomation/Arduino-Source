/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/StandardHid/StandardHid_Keyboard.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "InterruptableCommands.tpp"

namespace PokemonAutomation{


template class AsyncCommandSession<StandardHid::Keyboard>;
template class AsyncCommandSession<NintendoSwitch::ProController>;
template class AsyncCommandSession<NintendoSwitch::JoyconController>;


}
