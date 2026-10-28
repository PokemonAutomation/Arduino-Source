/*  Super-Control Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/StandardHid/StandardHid_Keyboard.h"

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "SuperControlSession.tpp"

namespace PokemonAutomation{


template class SuperControlSession<StandardHid::Keyboard>;
template class SuperControlSession<NintendoSwitch::ProController>;
template class SuperControlSession<NintendoSwitch::JoyconController>;


}
