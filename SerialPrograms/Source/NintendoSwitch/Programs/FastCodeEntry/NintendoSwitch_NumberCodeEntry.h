/*  Number Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_NumberCodeEntry_H
#define PokemonAutomation_NintendoSwitch_NumberCodeEntry_H

#include <string>
#include "Controllers/StandardHid/StandardHid_Keyboard.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{


//  Automatically select between keyboard or pro controller depending on the
//  controller type.
void numberpad_enter_code(
    ConsoleHandle& console, AbstractControllerContext& context,
    const std::string& code,
    bool include_plus
);

void numberpad_enter_code(
    ConsoleHandle& console, StandardHid::KeyboardContext& context,
    const std::string& code,
    bool include_plus
);
void numberpad_enter_code(
    ConsoleHandle& console, ProControllerContext& context,
    const std::string& code,
    bool include_plus
);


}
}
}
#endif
