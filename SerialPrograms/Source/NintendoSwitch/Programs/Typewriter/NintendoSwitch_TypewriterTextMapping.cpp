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

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch_TypewriterTextMapping.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace Typewriter{




const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY(KeyboardLayout layout){
    switch (layout){
    case KeyboardLayout::QWERTY:
        return TEXT_TO_HID_KEY_QWERTY();
    case KeyboardLayout::AZERTY:
        return TEXT_TO_HID_KEY_AZERTY();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Keyboard Type: " + std::to_string((int)layout)
        );
    }
}



//  US English (QWERTY) keyboard.
//
//  Every printable ASCII character can be typed with at most one Shift.
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_QWERTY(){
    static const std::map<char, TextKeyMapping> map{
        //  Letters
        {'a', {KeyboardKey::KEY_A,  false}},
        {'b', {KeyboardKey::KEY_B,  false}},
        {'c', {KeyboardKey::KEY_C,  false}},
        {'d', {KeyboardKey::KEY_D,  false}},
        {'e', {KeyboardKey::KEY_E,  false}},
        {'f', {KeyboardKey::KEY_F,  false}},
        {'g', {KeyboardKey::KEY_G,  false}},
        {'h', {KeyboardKey::KEY_H,  false}},
        {'i', {KeyboardKey::KEY_I,  false}},
        {'j', {KeyboardKey::KEY_J,  false}},
        {'k', {KeyboardKey::KEY_K,  false}},
        {'l', {KeyboardKey::KEY_L,  false}},
        {'m', {KeyboardKey::KEY_M,  false}},
        {'n', {KeyboardKey::KEY_N,  false}},
        {'o', {KeyboardKey::KEY_O,  false}},
        {'p', {KeyboardKey::KEY_P,  false}},
        {'q', {KeyboardKey::KEY_Q,  false}},
        {'r', {KeyboardKey::KEY_R,  false}},
        {'s', {KeyboardKey::KEY_S,  false}},
        {'t', {KeyboardKey::KEY_T,  false}},
        {'u', {KeyboardKey::KEY_U,  false}},
        {'v', {KeyboardKey::KEY_V,  false}},
        {'w', {KeyboardKey::KEY_W,  false}},
        {'x', {KeyboardKey::KEY_X,  false}},
        {'y', {KeyboardKey::KEY_Y,  false}},
        {'z', {KeyboardKey::KEY_Z,  false}},

        {'A', {KeyboardKey::KEY_A,  true}},
        {'B', {KeyboardKey::KEY_B,  true}},
        {'C', {KeyboardKey::KEY_C,  true}},
        {'D', {KeyboardKey::KEY_D,  true}},
        {'E', {KeyboardKey::KEY_E,  true}},
        {'F', {KeyboardKey::KEY_F,  true}},
        {'G', {KeyboardKey::KEY_G,  true}},
        {'H', {KeyboardKey::KEY_H,  true}},
        {'I', {KeyboardKey::KEY_I,  true}},
        {'J', {KeyboardKey::KEY_J,  true}},
        {'K', {KeyboardKey::KEY_K,  true}},
        {'L', {KeyboardKey::KEY_L,  true}},
        {'M', {KeyboardKey::KEY_M,  true}},
        {'N', {KeyboardKey::KEY_N,  true}},
        {'O', {KeyboardKey::KEY_O,  true}},
        {'P', {KeyboardKey::KEY_P,  true}},
        {'Q', {KeyboardKey::KEY_Q,  true}},
        {'R', {KeyboardKey::KEY_R,  true}},
        {'S', {KeyboardKey::KEY_S,  true}},
        {'T', {KeyboardKey::KEY_T,  true}},
        {'U', {KeyboardKey::KEY_U,  true}},
        {'V', {KeyboardKey::KEY_V,  true}},
        {'W', {KeyboardKey::KEY_W,  true}},
        {'X', {KeyboardKey::KEY_X,  true}},
        {'Y', {KeyboardKey::KEY_Y,  true}},
        {'Z', {KeyboardKey::KEY_Z,  true}},

        //  Numbers
        {'0', {KeyboardKey::KEY_0,  false}},
        {'1', {KeyboardKey::KEY_1,  false}},
        {'2', {KeyboardKey::KEY_2,  false}},
        {'3', {KeyboardKey::KEY_3,  false}},
        {'4', {KeyboardKey::KEY_4,  false}},
        {'5', {KeyboardKey::KEY_5,  false}},
        {'6', {KeyboardKey::KEY_6,  false}},
        {'7', {KeyboardKey::KEY_7,  false}},
        {'8', {KeyboardKey::KEY_8,  false}},
        {'9', {KeyboardKey::KEY_9,  false}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE,        false}},
        {'!',  {KeyboardKey::KEY_1,           true}},
        {'@',  {KeyboardKey::KEY_2,           true}},
        {'#',  {KeyboardKey::KEY_3,           true}},
        {'$',  {KeyboardKey::KEY_4,           true}},
        {'%',  {KeyboardKey::KEY_5,           true}},
        {'^',  {KeyboardKey::KEY_6,           true}},
        {'&',  {KeyboardKey::KEY_7,           true}},
        {'*',  {KeyboardKey::KEY_8,           true}},
        {'(',  {KeyboardKey::KEY_9,           true}},
        {')',  {KeyboardKey::KEY_0,           true}},
        {'-',  {KeyboardKey::KEY_MINUS,       false}},
        {'=',  {KeyboardKey::KEY_EQUAL,       false}},
        {'_',  {KeyboardKey::KEY_MINUS,       true}},
        {'+',  {KeyboardKey::KEY_EQUAL,       true}},
        {'[',  {KeyboardKey::KEY_LEFT_BRACE,  false}},
        {']',  {KeyboardKey::KEY_RIGHT_BRACE, false}},
        {'{',  {KeyboardKey::KEY_LEFT_BRACE,  true}},
        {'}',  {KeyboardKey::KEY_RIGHT_BRACE, true}},
        {'\\', {KeyboardKey::KEY_BACKSLASH,   false}},
        {'|',  {KeyboardKey::KEY_BACKSLASH,   true}},
        {';',  {KeyboardKey::KEY_SEMICOLON,   false}},
        {':',  {KeyboardKey::KEY_SEMICOLON,   true}},
        {'\'', {KeyboardKey::KEY_APOSTROPHE,  false}},
        {'"',  {KeyboardKey::KEY_APOSTROPHE,  true}},
        {'`',  {KeyboardKey::KEY_GRAVE,       false}},
        {'~',  {KeyboardKey::KEY_GRAVE,       true}},
        {',',  {KeyboardKey::KEY_COMMA,       false}},
        {'<',  {KeyboardKey::KEY_COMMA,       true}},
        {'.',  {KeyboardKey::KEY_DOT,         false}},
        {'>',  {KeyboardKey::KEY_DOT,         true}},
        {'/',  {KeyboardKey::KEY_SLASH,       false}},
        {'?',  {KeyboardKey::KEY_SLASH,       true}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER,       false}},
    };
    return map;
}



//  French (AZERTY) keyboard.
//
//  The HID key codes are assigned to physical key positions (per the US
//  layout), so to type "a" on an AZERTY keyboard we press the physical key
//  at the US "q" position, etc. This matches KEYBOARD_MAPPINGS_AZERTY in
//  NintendoSwitch_KeyboardEntryMappings.cpp.
//
//  Note that some ASCII characters cannot be typed on a French keyboard
//  with a single key press plus at most Shift: they require the AltGr
//  (right Alt) key. These are intentionally absent from this map:
//      #  <  >  @  [  ]  ^  `  {  }
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_AZERTY(){
    static const std::map<char, TextKeyMapping> map{
        //  Letters
        {'a', {KeyboardKey::KEY_Q,           false}},
        {'z', {KeyboardKey::KEY_W,           false}},
        {'e', {KeyboardKey::KEY_E,           false}},
        {'r', {KeyboardKey::KEY_R,           false}},
        {'t', {KeyboardKey::KEY_T,           false}},
        {'y', {KeyboardKey::KEY_Y,           false}},
        {'u', {KeyboardKey::KEY_U,           false}},
        {'i', {KeyboardKey::KEY_I,           false}},
        {'o', {KeyboardKey::KEY_O,           false}},
        {'p', {KeyboardKey::KEY_P,           false}},

        {'q', {KeyboardKey::KEY_A,           false}},
        {'s', {KeyboardKey::KEY_S,           false}},
        {'d', {KeyboardKey::KEY_D,           false}},
        {'f', {KeyboardKey::KEY_F,           false}},
        {'g', {KeyboardKey::KEY_G,           false}},
        {'h', {KeyboardKey::KEY_H,           false}},
        {'j', {KeyboardKey::KEY_J,           false}},
        {'k', {KeyboardKey::KEY_K,           false}},
        {'l', {KeyboardKey::KEY_L,           false}},
        {'m', {KeyboardKey::KEY_SEMICOLON,   false}},

        {'w', {KeyboardKey::KEY_Z,           false}},
        {'x', {KeyboardKey::KEY_X,           false}},
        {'c', {KeyboardKey::KEY_C,           false}},
        {'v', {KeyboardKey::KEY_V,           false}},
        {'b', {KeyboardKey::KEY_B,           false}},
        {'n', {KeyboardKey::KEY_N,           false}},

        {'A', {KeyboardKey::KEY_Q,           true}},
        {'Z', {KeyboardKey::KEY_W,           true}},
        {'E', {KeyboardKey::KEY_E,           true}},
        {'R', {KeyboardKey::KEY_R,           true}},
        {'T', {KeyboardKey::KEY_T,           true}},
        {'Y', {KeyboardKey::KEY_Y,           true}},
        {'U', {KeyboardKey::KEY_U,           true}},
        {'I', {KeyboardKey::KEY_I,           true}},
        {'O', {KeyboardKey::KEY_O,           true}},
        {'P', {KeyboardKey::KEY_P,           true}},

        {'Q', {KeyboardKey::KEY_A,           true}},
        {'S', {KeyboardKey::KEY_S,           true}},
        {'D', {KeyboardKey::KEY_D,           true}},
        {'F', {KeyboardKey::KEY_F,           true}},
        {'G', {KeyboardKey::KEY_G,           true}},
        {'H', {KeyboardKey::KEY_H,           true}},
        {'J', {KeyboardKey::KEY_J,           true}},
        {'K', {KeyboardKey::KEY_K,           true}},
        {'L', {KeyboardKey::KEY_L,           true}},
        {'M', {KeyboardKey::KEY_SEMICOLON,   true}},

        {'W', {KeyboardKey::KEY_Z,           true}},
        {'X', {KeyboardKey::KEY_X,           true}},
        {'C', {KeyboardKey::KEY_C,           true}},
        {'V', {KeyboardKey::KEY_V,           true}},
        {'B', {KeyboardKey::KEY_B,           true}},
        {'N', {KeyboardKey::KEY_N,           true}},

        //  Numbers (all require Shift on AZERTY)
        {'0', {KeyboardKey::KEY_0,  true}},
        {'1', {KeyboardKey::KEY_1,  true}},
        {'2', {KeyboardKey::KEY_2,  true}},
        {'3', {KeyboardKey::KEY_3,  true}},
        {'4', {KeyboardKey::KEY_4,  true}},
        {'5', {KeyboardKey::KEY_5,  true}},
        {'6', {KeyboardKey::KEY_6,  true}},
        {'7', {KeyboardKey::KEY_7,  true}},
        {'8', {KeyboardKey::KEY_8,  true}},
        {'9', {KeyboardKey::KEY_9,  true}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE,        false}},
        {'&',  {KeyboardKey::KEY_1,            false}},
        {'"',  {KeyboardKey::KEY_3,            false}},
        {'\'', {KeyboardKey::KEY_4,            false}},
        {'(',  {KeyboardKey::KEY_5,            false}},
        {'-',  {KeyboardKey::KEY_6,            false}},
        {'_',  {KeyboardKey::KEY_8,            false}},
        {')',  {KeyboardKey::KEY_MINUS,        false}},
        {'=',  {KeyboardKey::KEY_EQUAL,        false}},
        {'+',  {KeyboardKey::KEY_EQUAL,        true}},
        {'$',  {KeyboardKey::KEY_RIGHT_BRACE,  false}},
        {'\\', {KeyboardKey::KEY_BACKSLASH,    false}},
        {'|',  {KeyboardKey::KEY_BACKSLASH,    true}},
        {'%',  {KeyboardKey::KEY_APOSTROPHE,   true}},
        {'!',  {KeyboardKey::KEY_SLASH,        false}},
        {',',  {KeyboardKey::KEY_M,            false}},
        {'?',  {KeyboardKey::KEY_M,            true}},
        {';',  {KeyboardKey::KEY_COMMA,        false}},
        {'.',  {KeyboardKey::KEY_COMMA,        true}},
        {':',  {KeyboardKey::KEY_DOT,          false}},
        {'/',  {KeyboardKey::KEY_DOT,          true}},
        {'~',  {KeyboardKey::KEY_GRAVE,        true}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER,        false}},
    };
    return map;
}




}
}
}
