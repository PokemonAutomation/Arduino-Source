/*  BDSP AutoStory - Dialog Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <memory>
#include <string>

#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"

#include "PokemonBDSP_AutoStory_Dialog.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {


void clear_dialog(VideoStream& stream,
    ProControllerContext& context,
    ClearDialogMode mode,
    uint16_t timeout_sec,
    Button button,
    Milliseconds quiet_period,
    const char* instruction_card){
    context.wait_for_all_requests();
    std::unique_ptr<ImageMatchDetector> card;
    if (instruction_card){
        card = std::make_unique<ImageMatchDetector>(std::make_shared<ImageRGB32>(RESOURCE_PATH() + instruction_card),
            ImageFloatBox{.04, .32, .92, .34},
            30.0);
    }
    ShortDialogDetector dialogue(COLOR_YELLOW);
    BlackScreenDetector black;
    VideoOverlaySet overlays(stream.overlay());
    dialogue.make_overlays(overlays);
    black.make_overlays(overlays);
    std::unique_ptr<OverworldDetector> overworld;
    std::unique_ptr<ShortDialogPromptDetector> prompt;
    std::unique_ptr<SelectionArrowFinder> prompt_arrow;
    switch (mode){
    case ClearDialogMode::STOP_OVERWORLD:
        overworld = std::make_unique<OverworldDetector>();
        overworld->make_overlays(overlays);
        break;
    case ClearDialogMode::STOP_PROMPT:
        prompt = std::make_unique<ShortDialogPromptDetector>(stream.overlay(),
            ImageFloatBox{.60, .55, .30, .25}, COLOR_CYAN);
        prompt_arrow = std::make_unique<SelectionArrowFinder>(stream.overlay(),
            ImageFloatBox{.60, .55, .30, .25}, COLOR_CYAN);
        prompt->make_overlays(overlays);
        break;
    case ClearDialogMode::STOP_TIMEOUT:
    case ClearDialogMode::STOP_QUIET:
        break;
    default:
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Dialog: Unknown stop mode.",
            stream);
    }
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    auto absent_since = std::chrono::steady_clock::now();
    bool dialogue_seen = false;
    while (std::chrono::steady_clock::now() < deadline){
        const auto frame = stream.video().snapshot();
        if (!frame){
            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                "Dialog: No video frame.",
                stream);
        }
        const auto now = std::chrono::steady_clock::now();
        const bool dialogue_visible = dialogue.detect(*frame.frame);
        const bool card_visible = card && card->detect(*frame.frame);
        dialogue_seen = dialogue_seen || dialogue_visible || card_visible;
        if (prompt){
            if (prompt->process_frame(*frame.frame, frame.timestamp)){
                stream.log("[Dialog] Yes/No prompt confirmed; leaving the choice unanswered.");
                return;
            }
            // The prompt watcher needs five detections. Hold inputs as soon as
            // its arrow is visible so A cannot answer it during confirmation.
            if (dialogue_visible && prompt_arrow->detect(*frame.frame)){
                pbf_wait(context, 100ms);
                context.wait_for_all_requests();
                continue;
            }
        }
        const bool black_visible = black.detect(*frame.frame);
        const bool overworld_visible = overworld && overworld->detect(*frame.frame);
        if (dialogue_visible || card_visible || black_visible || (overworld && !overworld_visible)){
            absent_since = now;
        }else if (now - absent_since >= quiet_period &&
            (mode == ClearDialogMode::STOP_QUIET ||
                (mode == ClearDialogMode::STOP_OVERWORLD && dialogue_seen && overworld_visible))){
            stream.log(
                "[Dialog] " + std::to_string(quiet_period.count()) + "ms without dialogue confirmed.");
            return;
        }
        // A can interact again once dialogue disappears. Only confirm
        // visible prompts; B retains the existing post-battle behavior.
        if (card_visible){
            pbf_press_button(context, BUTTON_B, 80ms, 100ms);
        }else if (button == BUTTON_A && !dialogue_visible){
            pbf_wait(context, 100ms);
        }else if (mode == ClearDialogMode::STOP_PROMPT){
            // A mash can open and answer a choice between screenshots.
            // Advance once, release, then inspect before sending another input.
            pbf_press_button(context, button, 80ms, 700ms);
        }else{
            pbf_mash_button(context, button, 100ms);
        }
        context.wait_for_all_requests();
    }
    if (mode == ClearDialogMode::STOP_TIMEOUT){
        return;
    }
    if (mode == ClearDialogMode::STOP_PROMPT){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Dialog: Yes/No prompt not confirmed before timeout.",
            stream);
    }
    if (mode == ClearDialogMode::STOP_OVERWORLD){
        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
            "Dialog: Dialogue followed by a quiet overworld not confirmed before timeout.",
            stream);
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        "Dialog: Requested quiet period not confirmed before timeout.",
        stream);
}

void clear_dialog(VideoStream& stream,
    ProControllerContext& context,
    Button button,
    Milliseconds quiet_period,
    const char* instruction_card){
    clear_dialog(stream, context, ClearDialogMode::STOP_QUIET, 120, button, quiet_period, instruction_card);
}

void wait_for_dialog(VideoStream& stream,
    ProControllerContext& context,
    const std::string& label,
    Milliseconds timeout){
    context.wait_for_all_requests();
    ShortDialogWatcher dialogue(COLOR_CYAN);
    if (wait_until(stream, context, timeout, {{dialogue}}) == 0){
        if (!label.empty()){
            stream.log(label + " dialogue detected.");
        }
        return;
    }
    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
        (label.empty() ? "wait_for_dialog" : label) + ": dialogue not detected within timeout.",
        stream);
}

void wait_for_dialog(VideoStream& stream, ProControllerContext& context, uint16_t timeout_sec){
    wait_for_dialog(stream, context, "", std::chrono::seconds(timeout_sec));
}



}
}
}
