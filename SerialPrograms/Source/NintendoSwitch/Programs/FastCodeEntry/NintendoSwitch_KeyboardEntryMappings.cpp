/*  Keyboard Entry Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch_KeyboardEntryMappings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS(KeyboardLayout layout){
    switch (layout){
    case KeyboardLayout::QWERTY:
        return KEYBOARD_MAPPINGS_QWERTY();
    case KeyboardLayout::AZERTY:
        return KEYBOARD_MAPPINGS_AZERTY();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Keyboard Type: " + std::to_string((int)layout)
        );
    }
}
const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS(KeyboardLayout layout){
    switch (layout){
    case KeyboardLayout::QWERTY:
        return KEYBOARD_POSITIONS_QWERTY();
    case KeyboardLayout::AZERTY:
        return KEYBOARD_POSITIONS_AZERTY();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Keyboard Type: " + std::to_string((int)layout)
        );
    }
}





const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS_QWERTY(){
    using namespace StandardHid;
    static const std::map<char, KeyboardKey> map{
        {'1', KeyboardKey::KEY_KP_1},
        {'2', KeyboardKey::KEY_KP_2},
        {'3', KeyboardKey::KEY_KP_3},
        {'4', KeyboardKey::KEY_KP_4},
        {'5', KeyboardKey::KEY_KP_5},
        {'6', KeyboardKey::KEY_KP_6},
        {'7', KeyboardKey::KEY_KP_7},
        {'8', KeyboardKey::KEY_KP_8},
        {'9', KeyboardKey::KEY_KP_9},
        {'0', KeyboardKey::KEY_KP_0},

        {'Q', KeyboardKey::KEY_Q},
        {'W', KeyboardKey::KEY_W},
        {'E', KeyboardKey::KEY_E},
        {'R', KeyboardKey::KEY_R},
        {'T', KeyboardKey::KEY_T},
        {'Y', KeyboardKey::KEY_Y},
        {'U', KeyboardKey::KEY_U},
        {'I', KeyboardKey::KEY_I},
        {'O', KeyboardKey::KEY_O},
        {'P', KeyboardKey::KEY_P},

        {'A', KeyboardKey::KEY_A},
        {'S', KeyboardKey::KEY_S},
        {'D', KeyboardKey::KEY_D},
        {'F', KeyboardKey::KEY_F},
        {'G', KeyboardKey::KEY_G},
        {'H', KeyboardKey::KEY_H},
        {'J', KeyboardKey::KEY_J},
        {'K', KeyboardKey::KEY_K},
        {'L', KeyboardKey::KEY_L},

        {'Z', KeyboardKey::KEY_Z},
        {'X', KeyboardKey::KEY_X},
        {'C', KeyboardKey::KEY_C},
        {'V', KeyboardKey::KEY_V},
        {'B', KeyboardKey::KEY_B},
        {'N', KeyboardKey::KEY_N},
        {'M', KeyboardKey::KEY_M},
    };
    return map;
}
const std::map<char, StandardHid::KeyboardKey>& KEYBOARD_MAPPINGS_AZERTY(){
    using namespace StandardHid;
    static const std::map<char, KeyboardKey> map{
        {'1', KeyboardKey::KEY_KP_1},
        {'2', KeyboardKey::KEY_KP_2},
        {'3', KeyboardKey::KEY_KP_3},
        {'4', KeyboardKey::KEY_KP_4},
        {'5', KeyboardKey::KEY_KP_5},
        {'6', KeyboardKey::KEY_KP_6},
        {'7', KeyboardKey::KEY_KP_7},
        {'8', KeyboardKey::KEY_KP_8},
        {'9', KeyboardKey::KEY_KP_9},
        {'0', KeyboardKey::KEY_KP_0},

        {'A', KeyboardKey::KEY_Q},
        {'Z', KeyboardKey::KEY_W},
        {'E', KeyboardKey::KEY_E},
        {'R', KeyboardKey::KEY_R},
        {'T', KeyboardKey::KEY_T},
        {'Y', KeyboardKey::KEY_Y},
        {'U', KeyboardKey::KEY_U},
        {'I', KeyboardKey::KEY_I},
        {'O', KeyboardKey::KEY_O},
        {'P', KeyboardKey::KEY_P},

        {'Q', KeyboardKey::KEY_A},
        {'S', KeyboardKey::KEY_S},
        {'D', KeyboardKey::KEY_D},
        {'F', KeyboardKey::KEY_F},
        {'G', KeyboardKey::KEY_G},
        {'H', KeyboardKey::KEY_H},
        {'J', KeyboardKey::KEY_J},
        {'K', KeyboardKey::KEY_K},
        {'L', KeyboardKey::KEY_L},
        {'M', KeyboardKey::KEY_SEMICOLON},

        {'W', KeyboardKey::KEY_Z},
        {'X', KeyboardKey::KEY_X},
        {'C', KeyboardKey::KEY_C},
        {'V', KeyboardKey::KEY_V},
        {'B', KeyboardKey::KEY_B},
        {'N', KeyboardKey::KEY_N},
    };
    return map;
}




const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_QWERTY(){
    static const std::map<char, KeyboardEntryPosition> map{
        {'1', {0,  0}},
        {'2', {0,  1}},
        {'3', {0,  2}},
        {'4', {0,  3}},
        {'5', {0,  4}},
        {'6', {0,  5}},
        {'7', {0,  6}},
        {'8', {0,  7}},
        {'9', {0,  8}},
        {'0', {0,  9}},
        {'-', {0, 10}},

        {'Q', {1,  0}},
        {'W', {1,  1}},
        {'E', {1,  2}},
        {'R', {1,  3}},
        {'T', {1,  4}},
        {'Y', {1,  5}},
        {'U', {1,  6}},
        {'I', {1,  7}},
        {'O', {1,  8}},
        {'P', {1,  9}},
        {'/', {1, 10}},

        {'A', {2,  0}},
        {'S', {2,  1}},
        {'D', {2,  2}},
        {'F', {2,  3}},
        {'G', {2,  4}},
        {'H', {2,  5}},
        {'J', {2,  6}},
        {'K', {2,  7}},
        {'L', {2,  8}},
        {':', {2,  9}},
        {'\'', {2, 10}},

        {'Z', {3,  0}},
        {'X', {3,  1}},
        {'C', {3,  2}},
        {'V', {3,  3}},
        {'B', {3,  4}},
        {'N', {3,  5}},
        {'M', {3,  6}},
        {',', {3,  7}},
        {'.', {3,  8}},
        {'?', {3,  9}},
        {'!', {3, 10}},
    };
    return map;
}
const std::map<char, KeyboardEntryPosition>& KEYBOARD_POSITIONS_AZERTY(){
    static const std::map<char, KeyboardEntryPosition> map{
        {'1', {0,  0}},
        {'2', {0,  1}},
        {'3', {0,  2}},
        {'4', {0,  3}},
        {'5', {0,  4}},
        {'6', {0,  5}},
        {'7', {0,  6}},
        {'8', {0,  7}},
        {'9', {0,  8}},
        {'0', {0,  9}},
        {'-', {0, 10}},

        {'A', {1,  0}},
        {'Z', {1,  1}},
        {'E', {1,  2}},
        {'R', {1,  3}},
        {'T', {1,  4}},
        {'Y', {1,  5}},
        {'U', {1,  6}},
        {'I', {1,  7}},
        {'O', {1,  8}},
        {'P', {1,  9}},
        {'\'', {1, 10}},

        {'Q', {2, 0}},
        {'S', {2, 1}},
        {'D', {2, 2}},
        {'F', {2, 3}},
        {'G', {2, 4}},
        {'H', {2, 5}},
        {'J', {2, 6}},
        {'K', {2, 7}},
        {'L', {2, 8}},
        {'M', {2, 9}},
        {';', {2, 10}},

        {'W', {3,  0}},
        {'X', {3,  1}},
        {'C', {3,  2}},
        {'V', {3,  3}},
        {'B', {3,  4}},
        {'N', {3,  5}},
        {',', {3,  6}},
        {'.', {3,  7}},
        {'!', {3,  8}},
        {'?', {3,  9}},
        {':', {3, 10}},
    };
    return map;
}













}
}
