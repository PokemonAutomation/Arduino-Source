/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Soft reset, menus, etc.
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Navigation_H
#define PokemonAutomation_PokemonRSE_Navigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonRSE{

// Press A+B+Select+Start at the same time to soft reset, then re-enters the game.
// For now this assumes no dry battery.
void soft_reset(const ProgramInfo& info, VideoStream& stream, SwitchControllerContext &context);


}
}
}
#endif
