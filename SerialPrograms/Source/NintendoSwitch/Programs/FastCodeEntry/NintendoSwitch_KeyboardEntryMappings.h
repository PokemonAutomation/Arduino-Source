/*  Keyboard Entry Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardEntryMappings_H
#define PokemonAutomation_NintendoSwitch_KeyboardEntryMappings_H

#include <map>
#include "Controllers/StandardHid/StandardHid_Keyboard_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


enum class KeyboardLayout{
    QWERTY,
    AZERTY
};

const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS(KeyboardLayout layout);

const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS_QWERTY();
const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS_AZERTY();




struct KeyboardEntryPosition{
    uint8_t row;
    uint8_t col;
};

const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS(KeyboardLayout layout);

const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_QWERTY();
const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_AZERTY();






}
}
#endif
