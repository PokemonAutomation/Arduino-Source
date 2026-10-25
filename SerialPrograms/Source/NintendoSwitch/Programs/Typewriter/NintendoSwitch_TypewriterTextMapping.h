/*  Typewriter Text Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Maps printable ASCII characters to the raw HID key press(es) needed to
 *  type that character on a physical keyboard with a given layout. The
 *  layout is the one configured in the Switch's language settings.
 *
 *  Characters that cannot be typed on a given layout are simply absent
 *  from that layout's map.
 */

#ifndef PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H
#define PokemonAutomation_NintendoSwitch_TypewriterTextMapping_H

#include <map>
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
    ControlFlag control_flags;
};


//  Appends the modifier keys corresponding to "flags" to "keys", in the
//  order of the HID report modifier byte.
void control_flags_to_keys(ControlFlag flags, std::vector<KeyboardKey>& keys);


const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY(KeyboardLayout layout);
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_QWERTY();
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_AZERTY();



}
}
}
#endif
