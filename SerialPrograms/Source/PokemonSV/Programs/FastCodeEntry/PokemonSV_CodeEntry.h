/*  Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_CodeEntry_H
#define PokemonAutomation_PokemonSV_CodeEntry_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch/Programs/NintendoSwitch_FastCodeEntry.h"

namespace PokemonAutomation{
    class CancellableScope;
    class ProgramEnvironment;
namespace NintendoSwitch{
    class MultiSwitchProgramEnvironment;
namespace PokemonSV{


const char* normalize_code(std::string& normalized_code, const std::string& code, bool override_mode = false);

void enter_code(
    Logger& logger, SwitchControllerContext& context,
    const FastCodeEntrySettings& settings, const std::string& normalized_code,
    bool connect_controller_press,
    bool override_mode = false
);

const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings, const std::string& code,
    bool connect_controller_press,
    bool override_mode = false
);





}
}
}
#endif
