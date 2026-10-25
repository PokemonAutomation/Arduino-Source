/*  Typewriter Text Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Maps printable ASCII characters to the raw HID key press(es) needed to
 *  type that character on a physical keyboard with a given layout. The
 *  layout is the one configured in the Switch's language settings.
 *
 *  Characters that cannot be typed with a single key press (with at most
 *  Shift) on a given layout are simply absent from that layout's map.
 */

#ifndef PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H
#define PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H

#include <map>
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardEntryMappings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{


struct TextKeyMapping{
    KeyboardKey key;
    bool shift;
};


const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY(KeyboardLayout layout);
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_QWERTY();
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_AZERTY();



}
}
}
#endif
