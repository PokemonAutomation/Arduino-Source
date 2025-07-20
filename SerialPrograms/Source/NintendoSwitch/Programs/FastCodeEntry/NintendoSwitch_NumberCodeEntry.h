/*  Number Code Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_NumberCodeEntry_H
#define PokemonAutomation_NintendoSwitch_NumberCodeEntry_H

#include <string>
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace FastCodeEntry{



void numberpad_enter_code(
    ConsoleHandle& console, ProControllerContext& context,
    const std::string& code,
    bool include_plus
);



}
}
}
#endif
