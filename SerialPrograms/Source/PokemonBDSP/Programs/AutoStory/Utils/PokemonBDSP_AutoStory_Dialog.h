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

#include <chrono>
#include <cstdint>
#include <string>

#include "Common/Cpp/Time.h"
#include "CommonFramework/Tools/VideoStream.h"

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation {
struct ProgramInfo;
namespace NintendoSwitch {
namespace PokemonBDSP {


// ---------------------------------------------------------------------------
//  Stop conditions for clear_dialog()
// ---------------------------------------------------------------------------

enum class ClearDialogMode {
    //  Stop once the dialog has closed and the overworld is detected.
    STOP_OVERWORLD,

    //  Stop when a YES/NO prompt (ShortDialogPromptDetector) is detected.
    STOP_PROMPT,

    //  Continue clearing until `timeout_sec` expires, even if dialog closes.
    STOP_TIMEOUT,

    //  Stop after a quiet period without dialog, an instruction card, or a fade.
    //  Already-clear screens are accepted; this preserves existing route calls.
    STOP_QUIET,
};


// ---------------------------------------------------------------------------
//  Dialog clearing
// ---------------------------------------------------------------------------

//  Advance visible dialog with `button` until the selected stop condition.
//  STOP_OVERWORLD requires seeing dialog or an instruction card first, then a
//  continuous quiet overworld. STOP_PROMPT preserves the detected Yes/No choice.
//  STOP_TIMEOUT does not require any dialog to appear. Other modes throw if
//  their stop condition is not reached before `timeout_sec`; missing video fails.
void clear_dialog(VideoStream& stream,
    ProControllerContext& context,
    ClearDialogMode mode,
    uint16_t timeout_sec = 60,
    Button button = BUTTON_A,
    Milliseconds quiet_period = Milliseconds(3000),
    const char* instruction_card = nullptr);

// The button-based overload uses STOP_QUIET with a 120-second limit.
// Advance dialogue until the requested quiet period contains no dialogue,
// instruction card, or black screen. Used for story, item, and battle text.
void clear_dialog(VideoStream& stream,
    ProControllerContext& context,
    Button button = BUTTON_B,
    Milliseconds quiet_period = Milliseconds(3000),
    const char* instruction_card = nullptr);

// Wait without sending controller input until a short dialog box is confirmed.
// The timeout starts after pending controller requests finish. A missing dialog
// throws after one timeout window. The seconds-based API defaults to 30 seconds.
void wait_for_dialog(VideoStream& stream, ProControllerContext& context, uint16_t timeout_sec = 30);

// Labeled story-route calls retain their existing 60-second default and accept
// millisecond timeouts. An empty label suppresses the success message.
void wait_for_dialog(VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    Milliseconds timeout = std::chrono::seconds(60));



}
}
}
#endif
