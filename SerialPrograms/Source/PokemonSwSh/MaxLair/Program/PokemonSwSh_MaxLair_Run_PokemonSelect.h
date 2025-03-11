/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/FailureWatchdog.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker,
    OcrFailureWatchdog& ocr_watchdog,
    const ConsoleSpecificOptions& settings
);



}
}
}
}
#endif
