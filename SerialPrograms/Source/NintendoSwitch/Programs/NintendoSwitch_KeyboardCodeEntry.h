/*  Keyboard Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardCodeEntry_H
#define PokemonAutomation_NintendoSwitch_KeyboardCodeEntry_H

#include <string>
#include "NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{



void keyboard_enter_code(
    Logger& logger, ProControllerContext& context,
    KeyboardLayout keyboard_layout, const std::string& code,
    bool include_plus
);



}
}
#endif
