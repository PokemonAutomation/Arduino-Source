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

#include <cstdio>
#include <string>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
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



static std::string char_to_display(char ch){
    if (ch == '\n'){
        return "newline";
    }
    if (ch == '\t'){
        return "tab";
    }
    if (ch == '\r'){
        return "carriage return";
    }
    unsigned char uch = (unsigned char)ch;
    if (uch >= 0x20 && uch <= 0x7e){
        return std::string(1, ch);
    }
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "0x%02x", uch);
    return std::string(buffer);
}



std::vector<std::vector<KeyboardKey>> text_to_key_chords(
    KeyboardLayout layout,
    const std::string& text
){
    const std::map<char, TextKeyMapping>& mapping = TEXT_TO_HID_KEY(layout);

    std::vector<std::vector<KeyboardKey>> chords;
    chords.reserve(text.size());

    for (size_t i = 0; i < text.size(); i++){
        char ch = text[i];
        if (ch == '\r'){
            //  A carriage return is only valid as the first half of a CRLF
            //  pair (Windows line endings); the \n types the Enter.
            if (i + 1 == text.size() || text[i + 1] != '\n'){
                throw OperationFailedException(
                    ErrorReportMode::NO_ERROR_REPORT,
                    "Unsupported character: \"" + char_to_display(ch) +
                    "\". A standalone carriage return is not supported; "
                    "line endings must be CRLF."
                );
            }
            continue;
        }

        auto iter = mapping.find(ch);
        if (iter == mapping.end()){
            throw OperationFailedException(
                ErrorReportMode::NO_ERROR_REPORT,
                std::string("Unsupported character for the ") +
                (layout == KeyboardLayout::QWERTY ? "QWERTY" : "AZERTY") +
                " keyboard layout: \"" + char_to_display(ch) + "\". "
                "Only ASCII characters are supported, limited to "
                "printable characters, tab, and newline. Please use "
                "plain ASCII text."
            );
        }

        std::vector<KeyboardKey> keys;
        control_flags_to_keys(iter->second.control_flags, keys);
        keys.push_back(iter->second.key);
        chords.emplace_back(std::move(keys));
    }

    return chords;
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

        //  Control characters
        {'\t', {KeyboardKey::KEY_TAB}},
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
//  The character positions follow the standard modern French layout (the
//  one Windows uses). The characters that require the AltGr key are typed
//  with the right Alt, which is AltGr on a French keyboard, and the < > key
//  is the ISO key to the left of Space (HID 102nd key).
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
        {'%',  {KeyboardKey::KEY_APOSTROPHE, CONTROL_LEFT_SHIFT}},
        {'!',  {KeyboardKey::KEY_SLASH}},
        {',',  {KeyboardKey::KEY_M}},
        {'?',  {KeyboardKey::KEY_M, CONTROL_LEFT_SHIFT}},
        {';',  {KeyboardKey::KEY_COMMA}},
        {'.',  {KeyboardKey::KEY_COMMA, CONTROL_LEFT_SHIFT}},
        {':',  {KeyboardKey::KEY_DOT}},
        {'/',  {KeyboardKey::KEY_DOT, CONTROL_LEFT_SHIFT}},

        //  AltGr (right Alt) characters, by physical key
        {'~',  {KeyboardKey::KEY_2, CONTROL_RIGHT_ALT}},
        {'#',  {KeyboardKey::KEY_3, CONTROL_RIGHT_ALT}},
        {'{',  {KeyboardKey::KEY_4, CONTROL_RIGHT_ALT}},
        {'[',  {KeyboardKey::KEY_5, CONTROL_RIGHT_ALT}},
        {'|',  {KeyboardKey::KEY_6, CONTROL_RIGHT_ALT}},
        {'`',  {KeyboardKey::KEY_7, CONTROL_RIGHT_ALT}},
        {'\\', {KeyboardKey::KEY_8, CONTROL_RIGHT_ALT}},
        {'^',  {KeyboardKey::KEY_9, CONTROL_RIGHT_ALT}},
        {'@',  {KeyboardKey::KEY_0, CONTROL_RIGHT_ALT}},
        {']',  {KeyboardKey::KEY_MINUS, CONTROL_RIGHT_ALT}},
        {'}',  {KeyboardKey::KEY_EQUAL, CONTROL_RIGHT_ALT}},

        //  The < > key, to the left of Space on an ISO keyboard
        {'<',  {KeyboardKey::KEY_102ND}},
        {'>',  {KeyboardKey::KEY_102ND, CONTROL_LEFT_SHIFT}},

        //  Control characters
        {'\t', {KeyboardKey::KEY_TAB}},
        {'\n', {KeyboardKey::KEY_ENTER}},
    };
    return map;
}




}
}
}
