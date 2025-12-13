/*  HID Keyboard Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ControllerInput_KeyboardInput_KeyMappings_H
#define PokemonAutomation_ControllerInput_KeyboardInput_KeyMappings_H

#include <map>
#include "GlobalQtKeyMap.h"
#include "KeyboardHidButtons.h"

namespace PokemonAutomation{


const std::map<QtKeyMap::QtKey, KeyboardKey>& get_keyid_to_hid_map();

const std::map<Qt::Key, std::string>& QTKEY_TO_STRING();
const std::map<KeyboardKey, std::string>& KEYBOARDKEY_TO_STRING();

const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_QWERTY();
const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_AZERTY();



}
#endif
