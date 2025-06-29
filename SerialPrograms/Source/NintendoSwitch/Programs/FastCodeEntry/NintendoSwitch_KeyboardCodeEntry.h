/*  Keyboard Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardCodeEntry_H
#define PokemonAutomation_NintendoSwitch_KeyboardCodeEntry_H

#include <string>
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{



void keyboard_enter_code(
    ConsoleHandle& console, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
);



}
}
}
#endif
