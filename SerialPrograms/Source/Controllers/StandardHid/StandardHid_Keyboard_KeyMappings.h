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


const std::map<Qt::Key, std::string>& QTKEY_TO_STRING();
const std::map<KeyboardKey, std::string>& KEYBOARDKEY_TO_STRING();
const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_QWERTY();
const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_AZERTY();



}
}
#endif
