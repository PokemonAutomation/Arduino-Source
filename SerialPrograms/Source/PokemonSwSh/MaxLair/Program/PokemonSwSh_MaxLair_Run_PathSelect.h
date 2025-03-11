/*  Max Lair Run Path Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PathSelect_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_path_select(
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker
);


}
}
}
}
#endif
