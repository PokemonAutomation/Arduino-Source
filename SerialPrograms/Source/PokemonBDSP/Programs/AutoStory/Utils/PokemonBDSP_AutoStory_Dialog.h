/*  BDSP AutoStory - Dialog Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helpers for clearing NPC dialog and waiting for dialog to appear.
 *  These wrap the existing BDSP dialog detectors:
 *    - ShortDialogDetector / ShortDialogWatcher   (PokemonBDSP_DialogDetector.h)
 *    - ShortDialogPromptDetector                  (PokemonBDSP_DialogDetector.h)
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_Dialog_H
#define PokemonAutomation_PokemonBDSP_AutoStory_Dialog_H

#include <cstdint>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  Stop conditions for clear_dialog()
// ---------------------------------------------------------------------------

enum class ClearDialogMode{
    //  Stop once the dialog has closed and the overworld is detected.
    STOP_OVERWORLD,

    //  Stop when a YES/NO prompt (ShortDialogPromptDetector) is detected.
    STOP_PROMPT,

    //  Run for exactly `timeout_sec` seconds regardless of screen state.
    STOP_TIMEOUT,
};


// ---------------------------------------------------------------------------
//  Dialog clearing
// ---------------------------------------------------------------------------

//  Mash A through NPC dialog until `mode` stop condition is met.
//  Uses ShortDialogWatcher as the primary "still in dialog" signal.
//  Throws OperationFailedException if no dialog is ever detected, or if
//  the stop condition is not met within `timeout_sec` seconds.
void clear_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    ClearDialogMode mode,
    uint16_t timeout_sec = 60
);

//  Block until a short dialog box appears, then return.
//  Throws OperationFailedException if no dialog appears within `timeout_sec`.
void wait_for_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t timeout_sec = 30
);


}
}
}
#endif
