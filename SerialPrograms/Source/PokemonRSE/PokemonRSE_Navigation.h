/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Soft reset, menus, etc.
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Navigation_H
#define PokemonAutomation_PokemonRSE_Navigation_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonRSE{

// Press A+B+Select+Start at the same time to soft reset, then re-enters the game.
// This assumes no dry battery.
void soft_reset(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif
