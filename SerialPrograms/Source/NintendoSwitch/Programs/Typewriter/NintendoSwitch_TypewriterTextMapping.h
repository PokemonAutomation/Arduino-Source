/*  Typewriter Text Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Maps text characters (printable ASCII, tab, and newline) to the raw HID
 *  key press(es) needed to type that character on a physical keyboard with a
 *  given layout. The layout is the one configured in the Switch's language
 *  settings.
 *
 *  Characters that cannot be typed on a given layout are simply absent
 *  from that layout's map.
 */

#ifndef PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H
#define PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H

#include <map>
#include <string>
#include <vector>
#include "ControllerInput/Keyboard/KeyboardHidButtons.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardEntryMappings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{


//  The 8 modifier keys of the HID keyboard report's modifier byte.
//  The values match the byte exactly, so a ControlFlag (or an OR-combination
//  of them) can be stored directly as that byte.
enum ControlFlag : uint8_t{
    CONTROL_NONE          =   0x00,
    CONTROL_LEFT_CTRL     =   0x01,
    CONTROL_LEFT_SHIFT    =   0x02,
    CONTROL_LEFT_ALT      =   0x04,
    CONTROL_LEFT_META     =   0x08,
    CONTROL_RIGHT_CTRL    =   0x10,
    CONTROL_RIGHT_SHIFT   =   0x20,
    CONTROL_RIGHT_ALT     =   0x40,
    CONTROL_RIGHT_META    =   0x80,
};

inline constexpr ControlFlag empty_value(const ControlFlag*){
    return CONTROL_NONE;
}
inline constexpr bool is_empty(ControlFlag x){
    return x == CONTROL_NONE;
}
inline constexpr ControlFlag operator|(ControlFlag x, ControlFlag y){
    return (ControlFlag)((uint8_t)x | (uint8_t)y);
}
inline constexpr void operator|=(ControlFlag& x, ControlFlag y){
    x = (ControlFlag)((uint8_t)x | (uint8_t)y);
}
inline constexpr ControlFlag operator&(ControlFlag x, ControlFlag y){
    return (ControlFlag)((uint8_t)x & (uint8_t)y);
}
inline constexpr void operator&=(ControlFlag& x, ControlFlag y){
    x = (ControlFlag)((uint8_t)x & (uint8_t)y);
}
inline constexpr ControlFlag operator^(ControlFlag x, ControlFlag y){
    return (ControlFlag)((uint8_t)x ^ (uint8_t)y);
}
inline constexpr void operator^=(ControlFlag& x, ControlFlag y){
    x = (ControlFlag)((uint8_t)x ^ (uint8_t)y);
}


struct TextKeyMapping{
    KeyboardKey key;
    ControlFlag control_flags = ControlFlag::CONTROL_NONE;
};


//  Appends the modifier keys corresponding to "flags" to "keys", in the
//  order of the HID report modifier byte.
void control_flags_to_keys(ControlFlag flags, std::vector<KeyboardKey>& keys);


const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY(KeyboardLayout layout);
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_QWERTY();
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_AZERTY();


//  Converts "text" into the ordered list of key chords that types it on the
//  given layout. Each chord is the set of keys pressed simultaneously: the
//  modifier keys (in modifier-byte order) followed by the main key.
//
//  - Tab types Tab; a newline types Enter.
//  - A carriage return is only valid as the first half of a CRLF pair
//    (Windows line endings), in which case the \r is ignored and the \n types
//    the Enter. A lone \r is an error.
//
//  Throws OperationFailedException (NO_ERROR_REPORT) if the text contains a
//  character that cannot be typed on this layout.
std::vector<std::vector<KeyboardKey>> text_to_key_chords(
    KeyboardLayout layout,
    const std::string& text
);



}
}
}
#endif
