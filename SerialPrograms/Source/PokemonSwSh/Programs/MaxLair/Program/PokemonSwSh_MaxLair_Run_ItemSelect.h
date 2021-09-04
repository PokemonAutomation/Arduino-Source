/*  Max Lair Run Item Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "PokemonSwSh/Programs/MaxLair/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_item_select(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
