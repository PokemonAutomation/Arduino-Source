/*  Max Lair Run Item Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_item_select(
    ConsoleHandle& console, BotBaseContext& context,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
