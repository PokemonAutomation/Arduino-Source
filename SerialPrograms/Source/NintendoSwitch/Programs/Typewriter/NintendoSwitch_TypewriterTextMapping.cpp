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
        {'a', {KeyboardKey::KEY_A}},
        {'b', {KeyboardKey::KEY_B}},
        {'c', {KeyboardKey::KEY_C}},
        {'d', {KeyboardKey::KEY_D}},
        {'e', {KeyboardKey::KEY_E}},
        {'f', {KeyboardKey::KEY_F}},
        {'g', {KeyboardKey::KEY_G}},
        {'h', {KeyboardKey::KEY_H}},
        {'i', {KeyboardKey::KEY_I}},
        {'j', {KeyboardKey::KEY_J}},
        {'k', {KeyboardKey::KEY_K}},
        {'l', {KeyboardKey::KEY_L}},
        {'m', {KeyboardKey::KEY_M}},
        {'n', {KeyboardKey::KEY_N}},
        {'o', {KeyboardKey::KEY_O}},
        {'p', {KeyboardKey::KEY_P}},
        {'q', {KeyboardKey::KEY_Q}},
        {'r', {KeyboardKey::KEY_R}},
        {'s', {KeyboardKey::KEY_S}},
        {'t', {KeyboardKey::KEY_T}},
        {'u', {KeyboardKey::KEY_U}},
        {'v', {KeyboardKey::KEY_V}},
        {'w', {KeyboardKey::KEY_W}},
        {'x', {KeyboardKey::KEY_X}},
        {'y', {KeyboardKey::KEY_Y}},
        {'z', {KeyboardKey::KEY_Z}},

        {'A', {KeyboardKey::KEY_A, CONTROL_LEFT_SHIFT}},
        {'B', {KeyboardKey::KEY_B, CONTROL_LEFT_SHIFT}},
        {'C', {KeyboardKey::KEY_C, CONTROL_LEFT_SHIFT}},
        {'D', {KeyboardKey::KEY_D, CONTROL_LEFT_SHIFT}},
        {'E', {KeyboardKey::KEY_E, CONTROL_LEFT_SHIFT}},
        {'F', {KeyboardKey::KEY_F, CONTROL_LEFT_SHIFT}},
        {'G', {KeyboardKey::KEY_G, CONTROL_LEFT_SHIFT}},
        {'H', {KeyboardKey::KEY_H, CONTROL_LEFT_SHIFT}},
        {'I', {KeyboardKey::KEY_I, CONTROL_LEFT_SHIFT}},
        {'J', {KeyboardKey::KEY_J, CONTROL_LEFT_SHIFT}},
        {'K', {KeyboardKey::KEY_K, CONTROL_LEFT_SHIFT}},
        {'L', {KeyboardKey::KEY_L, CONTROL_LEFT_SHIFT}},
        {'M', {KeyboardKey::KEY_M, CONTROL_LEFT_SHIFT}},
        {'N', {KeyboardKey::KEY_N, CONTROL_LEFT_SHIFT}},
        {'O', {KeyboardKey::KEY_O, CONTROL_LEFT_SHIFT}},
        {'P', {KeyboardKey::KEY_P, CONTROL_LEFT_SHIFT}},
        {'Q', {KeyboardKey::KEY_Q, CONTROL_LEFT_SHIFT}},
        {'R', {KeyboardKey::KEY_R, CONTROL_LEFT_SHIFT}},
        {'S', {KeyboardKey::KEY_S, CONTROL_LEFT_SHIFT}},
        {'T', {KeyboardKey::KEY_T, CONTROL_LEFT_SHIFT}},
        {'U', {KeyboardKey::KEY_U, CONTROL_LEFT_SHIFT}},
        {'V', {KeyboardKey::KEY_V, CONTROL_LEFT_SHIFT}},
        {'W', {KeyboardKey::KEY_W, CONTROL_LEFT_SHIFT}},
        {'X', {KeyboardKey::KEY_X, CONTROL_LEFT_SHIFT}},
        {'Y', {KeyboardKey::KEY_Y, CONTROL_LEFT_SHIFT}},
        {'Z', {KeyboardKey::KEY_Z, CONTROL_LEFT_SHIFT}},

        //  Numbers
        {'0', {KeyboardKey::KEY_0}},
        {'1', {KeyboardKey::KEY_1}},
        {'2', {KeyboardKey::KEY_2}},
        {'3', {KeyboardKey::KEY_3}},
        {'4', {KeyboardKey::KEY_4}},
        {'5', {KeyboardKey::KEY_5}},
        {'6', {KeyboardKey::KEY_6}},
        {'7', {KeyboardKey::KEY_7}},
        {'8', {KeyboardKey::KEY_8}},
        {'9', {KeyboardKey::KEY_9}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE}},
        {'!',  {KeyboardKey::KEY_1, CONTROL_LEFT_SHIFT}},
        {'@',  {KeyboardKey::KEY_2, CONTROL_LEFT_SHIFT}},
        {'#',  {KeyboardKey::KEY_3, CONTROL_LEFT_SHIFT}},
        {'$',  {KeyboardKey::KEY_4, CONTROL_LEFT_SHIFT}},
        {'%',  {KeyboardKey::KEY_5, CONTROL_LEFT_SHIFT}},
        {'^',  {KeyboardKey::KEY_6, CONTROL_LEFT_SHIFT}},
        {'&',  {KeyboardKey::KEY_7, CONTROL_LEFT_SHIFT}},
        {'*',  {KeyboardKey::KEY_8, CONTROL_LEFT_SHIFT}},
        {'(',  {KeyboardKey::KEY_9, CONTROL_LEFT_SHIFT}},
        {')',  {KeyboardKey::KEY_0, CONTROL_LEFT_SHIFT}},
        {'-',  {KeyboardKey::KEY_MINUS}},
        {'=',  {KeyboardKey::KEY_EQUAL}},
        {'_',  {KeyboardKey::KEY_MINUS, CONTROL_LEFT_SHIFT}},
        {'+',  {KeyboardKey::KEY_EQUAL, CONTROL_LEFT_SHIFT}},
        {'[',  {KeyboardKey::KEY_LEFT_BRACE}},
        {']',  {KeyboardKey::KEY_RIGHT_BRACE}},
        {'{',  {KeyboardKey::KEY_LEFT_BRACE, CONTROL_LEFT_SHIFT}},
        {'}',  {KeyboardKey::KEY_RIGHT_BRACE, CONTROL_LEFT_SHIFT}},
        {'\\', {KeyboardKey::KEY_BACKSLASH}},
        {'|',  {KeyboardKey::KEY_BACKSLASH, CONTROL_LEFT_SHIFT}},
        {';',  {KeyboardKey::KEY_SEMICOLON}},
        {':',  {KeyboardKey::KEY_SEMICOLON, CONTROL_LEFT_SHIFT}},
        {'\'', {KeyboardKey::KEY_APOSTROPHE}},
        {'"',  {KeyboardKey::KEY_APOSTROPHE, CONTROL_LEFT_SHIFT}},
        {'`',  {KeyboardKey::KEY_GRAVE}},
        {'~',  {KeyboardKey::KEY_GRAVE, CONTROL_LEFT_SHIFT}},
        {',',  {KeyboardKey::KEY_COMMA}},
        {'<',  {KeyboardKey::KEY_COMMA, CONTROL_LEFT_SHIFT}},
        {'.',  {KeyboardKey::KEY_DOT}},
        {'>',  {KeyboardKey::KEY_DOT, CONTROL_LEFT_SHIFT}},
        {'/',  {KeyboardKey::KEY_SLASH}},
        {'?',  {KeyboardKey::KEY_SLASH, CONTROL_LEFT_SHIFT}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER}},
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
        {'a', {KeyboardKey::KEY_Q}},
        {'z', {KeyboardKey::KEY_W}},
        {'e', {KeyboardKey::KEY_E}},
        {'r', {KeyboardKey::KEY_R}},
        {'t', {KeyboardKey::KEY_T}},
        {'y', {KeyboardKey::KEY_Y}},
        {'u', {KeyboardKey::KEY_U}},
        {'i', {KeyboardKey::KEY_I}},
        {'o', {KeyboardKey::KEY_O}},
        {'p', {KeyboardKey::KEY_P}},

        {'q', {KeyboardKey::KEY_A}},
        {'s', {KeyboardKey::KEY_S}},
        {'d', {KeyboardKey::KEY_D}},
        {'f', {KeyboardKey::KEY_F}},
        {'g', {KeyboardKey::KEY_G}},
        {'h', {KeyboardKey::KEY_H}},
        {'j', {KeyboardKey::KEY_J}},
        {'k', {KeyboardKey::KEY_K}},
        {'l', {KeyboardKey::KEY_L}},
        {'m', {KeyboardKey::KEY_SEMICOLON}},

        {'w', {KeyboardKey::KEY_Z}},
        {'x', {KeyboardKey::KEY_X}},
        {'c', {KeyboardKey::KEY_C}},
        {'v', {KeyboardKey::KEY_V}},
        {'b', {KeyboardKey::KEY_B}},
        {'n', {KeyboardKey::KEY_N}},

        {'A', {KeyboardKey::KEY_Q, CONTROL_LEFT_SHIFT}},
        {'Z', {KeyboardKey::KEY_W, CONTROL_LEFT_SHIFT}},
        {'E', {KeyboardKey::KEY_E, CONTROL_LEFT_SHIFT}},
        {'R', {KeyboardKey::KEY_R, CONTROL_LEFT_SHIFT}},
        {'T', {KeyboardKey::KEY_T, CONTROL_LEFT_SHIFT}},
        {'Y', {KeyboardKey::KEY_Y, CONTROL_LEFT_SHIFT}},
        {'U', {KeyboardKey::KEY_U, CONTROL_LEFT_SHIFT}},
        {'I', {KeyboardKey::KEY_I, CONTROL_LEFT_SHIFT}},
        {'O', {KeyboardKey::KEY_O, CONTROL_LEFT_SHIFT}},
        {'P', {KeyboardKey::KEY_P, CONTROL_LEFT_SHIFT}},

        {'Q', {KeyboardKey::KEY_A, CONTROL_LEFT_SHIFT}},
        {'S', {KeyboardKey::KEY_S, CONTROL_LEFT_SHIFT}},
        {'D', {KeyboardKey::KEY_D, CONTROL_LEFT_SHIFT}},
        {'F', {KeyboardKey::KEY_F, CONTROL_LEFT_SHIFT}},
        {'G', {KeyboardKey::KEY_G, CONTROL_LEFT_SHIFT}},
        {'H', {KeyboardKey::KEY_H, CONTROL_LEFT_SHIFT}},
        {'J', {KeyboardKey::KEY_J, CONTROL_LEFT_SHIFT}},
        {'K', {KeyboardKey::KEY_K, CONTROL_LEFT_SHIFT}},
        {'L', {KeyboardKey::KEY_L, CONTROL_LEFT_SHIFT}},
        {'M', {KeyboardKey::KEY_SEMICOLON, CONTROL_LEFT_SHIFT}},

        {'W', {KeyboardKey::KEY_Z, CONTROL_LEFT_SHIFT}},
        {'X', {KeyboardKey::KEY_X, CONTROL_LEFT_SHIFT}},
        {'C', {KeyboardKey::KEY_C, CONTROL_LEFT_SHIFT}},
        {'V', {KeyboardKey::KEY_V, CONTROL_LEFT_SHIFT}},
        {'B', {KeyboardKey::KEY_B, CONTROL_LEFT_SHIFT}},
        {'N', {KeyboardKey::KEY_N, CONTROL_LEFT_SHIFT}},

        //  Numbers (all require Shift on AZERTY)
        {'0', {KeyboardKey::KEY_0, CONTROL_LEFT_SHIFT}},
        {'1', {KeyboardKey::KEY_1, CONTROL_LEFT_SHIFT}},
        {'2', {KeyboardKey::KEY_2, CONTROL_LEFT_SHIFT}},
        {'3', {KeyboardKey::KEY_3, CONTROL_LEFT_SHIFT}},
        {'4', {KeyboardKey::KEY_4, CONTROL_LEFT_SHIFT}},
        {'5', {KeyboardKey::KEY_5, CONTROL_LEFT_SHIFT}},
        {'6', {KeyboardKey::KEY_6, CONTROL_LEFT_SHIFT}},
        {'7', {KeyboardKey::KEY_7, CONTROL_LEFT_SHIFT}},
        {'8', {KeyboardKey::KEY_8, CONTROL_LEFT_SHIFT}},
        {'9', {KeyboardKey::KEY_9, CONTROL_LEFT_SHIFT}},

        //  Punctuation
        {' ',  {KeyboardKey::KEY_SPACE}},
        {'&',  {KeyboardKey::KEY_1}},
        {'"',  {KeyboardKey::KEY_3}},
        {'\'', {KeyboardKey::KEY_4}},
        {'(',  {KeyboardKey::KEY_5}},
        {'-',  {KeyboardKey::KEY_6}},
        {'_',  {KeyboardKey::KEY_8}},
        {')',  {KeyboardKey::KEY_MINUS}},
        {'=',  {KeyboardKey::KEY_EQUAL}},
        {'+',  {KeyboardKey::KEY_EQUAL, CONTROL_LEFT_SHIFT}},
        {'$',  {KeyboardKey::KEY_RIGHT_BRACE}},
        {'*',  {KeyboardKey::KEY_BACKSLASH}},
        {'\\', {KeyboardKey::KEY_BACKSLASH}},
        {'|',  {KeyboardKey::KEY_BACKSLASH, CONTROL_LEFT_SHIFT}},
        {'%',  {KeyboardKey::KEY_APOSTROPHE, CONTROL_LEFT_SHIFT}},
        {'!',  {KeyboardKey::KEY_SLASH}},
        {',',  {KeyboardKey::KEY_M}},
        {'?',  {KeyboardKey::KEY_M, CONTROL_LEFT_SHIFT}},
        {';',  {KeyboardKey::KEY_COMMA}},
        {'.',  {KeyboardKey::KEY_COMMA, CONTROL_LEFT_SHIFT}},
        {':',  {KeyboardKey::KEY_DOT}},
        {'/',  {KeyboardKey::KEY_DOT, CONTROL_LEFT_SHIFT}},
        {'~',  {KeyboardKey::KEY_GRAVE, CONTROL_LEFT_SHIFT}},

        //  Newline
        {'\n', {KeyboardKey::KEY_ENTER}},
    };
    return map;
}




}
}
}
