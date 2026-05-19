/*  BDSP AutoStory - Dialog Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP_AutoStory_Dialog.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void clear_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    ClearDialogMode mode,
    uint16_t timeout_sec
){
    // TODO: implement
    //  - use ShortDialogWatcher + (optionally) ShortDialogPromptDetector
    //    + OverworldDetector (from Detect/) with run_until<>
    //  - loop mashing A until stop condition fires
    (void)stream; (void)context; (void)mode; (void)timeout_sec;
}

void wait_for_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t timeout_sec
){
    // TODO: implement
    //  - use wait_until() with ShortDialogWatcher
    (void)stream; (void)context; (void)timeout_sec;
}


}
}
}
