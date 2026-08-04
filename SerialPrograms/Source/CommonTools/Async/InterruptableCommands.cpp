/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "InterruptableCommands.tpp"

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{


template class AsyncCommandSession<NintendoSwitch::ProController>;
template class AsyncCommandSession<NintendoSwitch::JoyconController>;


}
