/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SuperControlSession.tpp"

#include "Controllers/StandardHid/StandardHid_Keyboard.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{


template class SuperControlSession<StandardHid::Keyboard>;
template class SuperControlSession<NintendoSwitch::ProController>;
template class SuperControlSession<NintendoSwitch::JoyconController>;


}
