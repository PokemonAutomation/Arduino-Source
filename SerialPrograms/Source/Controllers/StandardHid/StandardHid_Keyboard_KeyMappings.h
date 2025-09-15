/*  HID Keyboard Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_KeyMappings_H
#define PokemonAutomation_StandardHid_Keyboard_KeyMappings_H

#include <map>
#include "Controllers/KeyboardInput/GlobalQtKeyMap.h"
#include "StandardHid_Keyboard_ControllerButtons.h"

namespace PokemonAutomation{
namespace StandardHid{


const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_QWERTY();



}
}
#endif
