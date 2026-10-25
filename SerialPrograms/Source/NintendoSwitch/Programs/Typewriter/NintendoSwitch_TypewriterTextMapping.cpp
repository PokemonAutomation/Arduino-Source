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



void control_flags_to_keys(ControlFlag flags, std::vector<KeyboardKey>& keys){
    //  In the order of the HID report modifier byte.
    if ((flags & CONTROL_LEFT_CTRL) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_LEFT_CTRL);
    }
    if ((flags & CONTROL_LEFT_SHIFT) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_LEFT_SHIFT);
    }
    if ((flags & CONTROL_LEFT_ALT) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_LEFT_ALT);
    }
    if ((flags & CONTROL_LEFT_META) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_LEFT_META);
    }
    if ((flags & CONTROL_RIGHT_CTRL) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_RIGHT_CTRL);
    }
    if ((flags & CONTROL_RIGHT_SHIFT) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_RIGHT_SHIFT);
    }
    if ((flags & CONTROL_RIGHT_ALT) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_RIGHT_ALT);
    }
    if ((flags & CONTROL_RIGHT_META) != CONTROL_NONE){
        keys.push_back(KeyboardKey::KEY_RIGHT_META);
    }
}



//  US English (QWERTY) keyboard.
//
//  Every printable ASCII character can be typed with at most one Shift.
const std::map<char, TextKeyMapping>& TEXT_TO_HID_KEY_QWERTY(){
    static const std::map<char, TextKeyMapping> map{
        //  Letters
        {'a', {KeyboardKey::KEY_A, ControlFlag::CONTROL_NONE}},
        {'b', {KeyboardKey::KEY_B, ControlFlag::CONTROL_NONE}},
        {'c', {KeyboardKey::KEY_C, ControlFlag::CONTROL_NONE}},
        {'d', {KeyboardKey::KEY_D, ControlFlag::CONTROL_NONE}},
        {'e', {KeyboardKey::KEY_E, ControlFlag::CONTROL_NONE}},
        {'f', {KeyboardKey::KEY_F, ControlFlag::CONTROL_NONE}},
        {'g', {KeyboardKey::KEY_G, ControlFlag::CONTROL_NONE}},
        {'h', {KeyboardKey::KEY_H, ControlFlag::CONTROL_NONE}},
        {'i', {KeyboardKey::KEY_I, ControlFlag::CONTROL_NONE}},
        {'j', {KeyboardKey::KEY_J, ControlFlag::CONTROL_NONE}},
        {'k', {KeyboardKey::KEY_K, ControlFlag::CONTROL_NONE}},
        {'l', {KeyboardKey::KEY_L, ControlFlag::CONTROL_NONE}},
        {'m', {KeyboardKey::KEY_M, ControlFlag::CONTROL_NONE}},
        {'n', {KeyboardKey::KEY_N, ControlFlag::CONTROL_NONE}},
        {'o', {KeyboardKey::KEY_O, ControlFlag::CONTROL_NONE}},
        {'p', {KeyboardKey::KEY_P, ControlFlag::CONTROL_NONE}},
        {'q', {KeyboardKey::KEY_Q, ControlFlag::CONTROL_NONE}},
        {'r', {KeyboardKey::KEY_R, ControlFlag::CONTROL_NONE}},
        {'s', {KeyboardKey::KEY_S, ControlFlag::CONTROL_NONE}},
        {'t', {KeyboardKey::KEY_T, ControlFlag::CONTROL_NONE}},
        {'u', {KeyboardKey::KEY_U, ControlFlag::CONTROL_NONE}},
        {'v', {KeyboardKey::KEY_V, ControlFlag::CONTROL_NONE}},
        {'w', {KeyboardKey::KEY_W, ControlFlag::CONTROL_NONE}},
        {'x', {KeyboardKey::KEY_X, ControlFlag::CONTROL_NONE}},
        {'y', {KeyboardKey::KEY_Y, ControlFlag::CONTROL_NONE}},
        {'z', {KeyboardKey::KEY_Z, ControlFlag::CONTROL_NONE}},

        {'A', {KeyboardKey::KEY_A, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'B', {KeyboardKey::KEY_B, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'C', {KeyboardKey::KEY_C, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'D', {KeyboardKey::KEY_D, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'E', {KeyboardKey::KEY_E, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'F', {KeyboardKey::KEY_F, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'G', {KeyboardKey::KEY_G, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'H', {KeyboardKey::KEY_H, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'I', {KeyboardKey::KEY_I, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'J', {KeyboardKey::KEY_J, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'K', {KeyboardKey::KEY_K, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'L', {KeyboardKey::KEY_L, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'M', {KeyboardKey::KEY_M, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'N', {KeyboardKey::KEY_N, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'O', {KeyboardKey::KEY_O, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'P', {KeyboardKey::KEY_P, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'Q', {KeyboardKey::KEY_Q, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'R', {KeyboardKey::KEY_R, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'S', {KeyboardKey::KEY_S, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'T', {KeyboardKey::KEY_T, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'U', {KeyboardKey::KEY_U, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'V', {KeyboardKey::KEY_V, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'W', {KeyboardKey::KEY_W, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'X', {KeyboardKey::KEY_X, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'Y', {KeyboardKey::KEY_Y, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'Z', {KeyboardKey::KEY_Z, ControlFlag::CONTROL_LEFT_SHIFT}},

        //  Numbers
        {'0', {KeyboardKey::KEY_0, ControlFlag::CONTROL_NONE}},
        {'1', {KeyboardKey::KEY_1, ControlFlag::CONTROL_NONE}},
        {'2', {KeyboardKey::KEY_2, ControlFlag::CONTROL_NONE}},
        {'3', {KeyboardKey::KEY_3, ControlFlag::CONTROL_NONE}},
        {'4', {KeyboardKey::KEY_4, ControlFlag::CONTROL_NONE}},
        {'5', {KeyboardKey::KEY_5, ControlFlag::CONTROL_NONE}},
        {'6', {KeyboardKey::KEY_6, ControlFlag::CONTROL_NONE}},
        {'7', {KeyboardKey::KEY_7, ControlFlag::CONTROL_NONE}},
        {'8', {KeyboardKey::KEY_8, ControlFlag::CONTROL_NONE}},
        {'9', {KeyboardKey::KEY_9, ControlFlag::CONTROL_NONE}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE, ControlFlag::CONTROL_NONE}},
        {'!',  {KeyboardKey::KEY_1, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'@',  {KeyboardKey::KEY_2, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'#',  {KeyboardKey::KEY_3, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'$',  {KeyboardKey::KEY_4, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'%',  {KeyboardKey::KEY_5, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'^',  {KeyboardKey::KEY_6, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'&',  {KeyboardKey::KEY_7, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'*',  {KeyboardKey::KEY_8, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'(',  {KeyboardKey::KEY_9, ControlFlag::CONTROL_LEFT_SHIFT}},
        {')',  {KeyboardKey::KEY_0, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'-',  {KeyboardKey::KEY_MINUS, ControlFlag::CONTROL_NONE}},
        {'=',  {KeyboardKey::KEY_EQUAL, ControlFlag::CONTROL_NONE}},
        {'_',  {KeyboardKey::KEY_MINUS, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'+',  {KeyboardKey::KEY_EQUAL, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'[',  {KeyboardKey::KEY_LEFT_BRACE, ControlFlag::CONTROL_NONE}},
        {']',  {KeyboardKey::KEY_RIGHT_BRACE, ControlFlag::CONTROL_NONE}},
        {'{',  {KeyboardKey::KEY_LEFT_BRACE, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'}',  {KeyboardKey::KEY_RIGHT_BRACE, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'\\', {KeyboardKey::KEY_BACKSLASH, ControlFlag::CONTROL_NONE}},
        {'|',  {KeyboardKey::KEY_BACKSLASH, ControlFlag::CONTROL_LEFT_SHIFT}},
        {';',  {KeyboardKey::KEY_SEMICOLON, ControlFlag::CONTROL_NONE}},
        {':',  {KeyboardKey::KEY_SEMICOLON, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'\'', {KeyboardKey::KEY_APOSTROPHE, ControlFlag::CONTROL_NONE}},
        {'"',  {KeyboardKey::KEY_APOSTROPHE, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'`',  {KeyboardKey::KEY_GRAVE, ControlFlag::CONTROL_NONE}},
        {'~',  {KeyboardKey::KEY_GRAVE, ControlFlag::CONTROL_LEFT_SHIFT}},
        {',',  {KeyboardKey::KEY_COMMA, ControlFlag::CONTROL_NONE}},
        {'<',  {KeyboardKey::KEY_COMMA, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'.',  {KeyboardKey::KEY_DOT, ControlFlag::CONTROL_NONE}},
        {'>',  {KeyboardKey::KEY_DOT, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'/',  {KeyboardKey::KEY_SLASH, ControlFlag::CONTROL_NONE}},
        {'?',  {KeyboardKey::KEY_SLASH, ControlFlag::CONTROL_LEFT_SHIFT}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER, ControlFlag::CONTROL_NONE}},
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
        {'a', {KeyboardKey::KEY_Q, ControlFlag::CONTROL_NONE}},
        {'z', {KeyboardKey::KEY_W, ControlFlag::CONTROL_NONE}},
        {'e', {KeyboardKey::KEY_E, ControlFlag::CONTROL_NONE}},
        {'r', {KeyboardKey::KEY_R, ControlFlag::CONTROL_NONE}},
        {'t', {KeyboardKey::KEY_T, ControlFlag::CONTROL_NONE}},
        {'y', {KeyboardKey::KEY_Y, ControlFlag::CONTROL_NONE}},
        {'u', {KeyboardKey::KEY_U, ControlFlag::CONTROL_NONE}},
        {'i', {KeyboardKey::KEY_I, ControlFlag::CONTROL_NONE}},
        {'o', {KeyboardKey::KEY_O, ControlFlag::CONTROL_NONE}},
        {'p', {KeyboardKey::KEY_P, ControlFlag::CONTROL_NONE}},

        {'q', {KeyboardKey::KEY_A, ControlFlag::CONTROL_NONE}},
        {'s', {KeyboardKey::KEY_S, ControlFlag::CONTROL_NONE}},
        {'d', {KeyboardKey::KEY_D, ControlFlag::CONTROL_NONE}},
        {'f', {KeyboardKey::KEY_F, ControlFlag::CONTROL_NONE}},
        {'g', {KeyboardKey::KEY_G, ControlFlag::CONTROL_NONE}},
        {'h', {KeyboardKey::KEY_H, ControlFlag::CONTROL_NONE}},
        {'j', {KeyboardKey::KEY_J, ControlFlag::CONTROL_NONE}},
        {'k', {KeyboardKey::KEY_K, ControlFlag::CONTROL_NONE}},
        {'l', {KeyboardKey::KEY_L, ControlFlag::CONTROL_NONE}},
        {'m', {KeyboardKey::KEY_SEMICOLON, ControlFlag::CONTROL_NONE}},

        {'w', {KeyboardKey::KEY_Z, ControlFlag::CONTROL_NONE}},
        {'x', {KeyboardKey::KEY_X, ControlFlag::CONTROL_NONE}},
        {'c', {KeyboardKey::KEY_C, ControlFlag::CONTROL_NONE}},
        {'v', {KeyboardKey::KEY_V, ControlFlag::CONTROL_NONE}},
        {'b', {KeyboardKey::KEY_B, ControlFlag::CONTROL_NONE}},
        {'n', {KeyboardKey::KEY_N, ControlFlag::CONTROL_NONE}},

        {'A', {KeyboardKey::KEY_Q, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'Z', {KeyboardKey::KEY_W, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'E', {KeyboardKey::KEY_E, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'R', {KeyboardKey::KEY_R, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'T', {KeyboardKey::KEY_T, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'Y', {KeyboardKey::KEY_Y, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'U', {KeyboardKey::KEY_U, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'I', {KeyboardKey::KEY_I, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'O', {KeyboardKey::KEY_O, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'P', {KeyboardKey::KEY_P, ControlFlag::CONTROL_LEFT_SHIFT}},

        {'Q', {KeyboardKey::KEY_A, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'S', {KeyboardKey::KEY_S, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'D', {KeyboardKey::KEY_D, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'F', {KeyboardKey::KEY_F, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'G', {KeyboardKey::KEY_G, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'H', {KeyboardKey::KEY_H, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'J', {KeyboardKey::KEY_J, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'K', {KeyboardKey::KEY_K, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'L', {KeyboardKey::KEY_L, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'M', {KeyboardKey::KEY_SEMICOLON, ControlFlag::CONTROL_LEFT_SHIFT}},

        {'W', {KeyboardKey::KEY_Z, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'X', {KeyboardKey::KEY_X, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'C', {KeyboardKey::KEY_C, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'V', {KeyboardKey::KEY_V, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'B', {KeyboardKey::KEY_B, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'N', {KeyboardKey::KEY_N, ControlFlag::CONTROL_LEFT_SHIFT}},

        //  Numbers (all require Shift on AZERTY)
        {'0', {KeyboardKey::KEY_0, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'1', {KeyboardKey::KEY_1, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'2', {KeyboardKey::KEY_2, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'3', {KeyboardKey::KEY_3, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'4', {KeyboardKey::KEY_4, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'5', {KeyboardKey::KEY_5, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'6', {KeyboardKey::KEY_6, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'7', {KeyboardKey::KEY_7, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'8', {KeyboardKey::KEY_8, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'9', {KeyboardKey::KEY_9, ControlFlag::CONTROL_LEFT_SHIFT}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE, ControlFlag::CONTROL_NONE}},
        {'&',  {KeyboardKey::KEY_1, ControlFlag::CONTROL_NONE}},
        {'"',  {KeyboardKey::KEY_3, ControlFlag::CONTROL_NONE}},
        {'\'', {KeyboardKey::KEY_4, ControlFlag::CONTROL_NONE}},
        {'(',  {KeyboardKey::KEY_5, ControlFlag::CONTROL_NONE}},
        {'-',  {KeyboardKey::KEY_6, ControlFlag::CONTROL_NONE}},
        {'_',  {KeyboardKey::KEY_8, ControlFlag::CONTROL_NONE}},
        {')',  {KeyboardKey::KEY_MINUS, ControlFlag::CONTROL_NONE}},
        {'=',  {KeyboardKey::KEY_EQUAL, ControlFlag::CONTROL_NONE}},
        {'+',  {KeyboardKey::KEY_EQUAL, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'$',  {KeyboardKey::KEY_RIGHT_BRACE, ControlFlag::CONTROL_NONE}},
        {'\\', {KeyboardKey::KEY_BACKSLASH, ControlFlag::CONTROL_NONE}},
        {'|',  {KeyboardKey::KEY_BACKSLASH, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'%',  {KeyboardKey::KEY_APOSTROPHE, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'!',  {KeyboardKey::KEY_SLASH, ControlFlag::CONTROL_NONE}},
        {',',  {KeyboardKey::KEY_M, ControlFlag::CONTROL_NONE}},
        {'?',  {KeyboardKey::KEY_M, ControlFlag::CONTROL_LEFT_SHIFT}},
        {';',  {KeyboardKey::KEY_COMMA, ControlFlag::CONTROL_NONE}},
        {'.',  {KeyboardKey::KEY_COMMA, ControlFlag::CONTROL_LEFT_SHIFT}},
        {':',  {KeyboardKey::KEY_DOT, ControlFlag::CONTROL_NONE}},
        {'/',  {KeyboardKey::KEY_DOT, ControlFlag::CONTROL_LEFT_SHIFT}},
        {'~',  {KeyboardKey::KEY_GRAVE, ControlFlag::CONTROL_LEFT_SHIFT}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER, ControlFlag::CONTROL_NONE}},
    };
    return map;
}




}
}
}
