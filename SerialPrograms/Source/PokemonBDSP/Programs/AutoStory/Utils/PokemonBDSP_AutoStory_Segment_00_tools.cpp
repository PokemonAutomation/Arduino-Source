/*  BDSP AutoStory - Segment 00 Tools
 *
 *  Helper implementations for Segment 00.
 * 
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "PokemonBDSP/Inference/PokemonBDSP_TransitionWatchers.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cctype>

#include "PokemonBDSP_AutoStory_Segment_00_tools.h"
#include "PokemonBDSP_Autostory_Segment_01_tools.h"

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"

#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"

#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"

#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ReceivePokemonDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"

#include "PokemonBDSP/Programs/AutoStory/Detect/PokemonBDSP_AutoStory_OverworldDetector.h"

#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonBDSP {

            // ---------------------------------------------------------------------------
            // BattleDialogWatcher
            // ---------------------------------------------------------------------------

            BattleDialogWatcher::BattleDialogWatcher(Color color)
                : DetectorToFinder(
                    "BattleDialogWatcher",
                    std::chrono::milliseconds(250),
                    color
                )
            {
            }

            // ---------------------------------------------------------------------------
            // Poketech Clown dialog helper
            // ---------------------------------------------------------------------------

            int detect_clown_choice_arrow(
                VideoOverlay& overlay,
                VideoSnapshot snapshot
            ) {
                SelectionArrowFinder top_arrow(
                    overlay,
                    { 0.62, 0.37, 0.08, 0.10 },
                    COLOR_RED
                );

                SelectionArrowFinder bottom_arrow(
                    overlay,
                    { 0.62, 0.48, 0.08, 0.10 },
                    COLOR_RED
                );

                if (top_arrow.detect(snapshot)) {
                    return 0;
                }

                if (bottom_arrow.detect(snapshot)) {
                    return 1;
                }

                return -1;
            }

            void clown_dialog_helper(
                VideoStream& stream,
                ProControllerContext& context,
                DpadPosition direction
            ) {
                stream.log(
                    "[AutoStory] Starting clown dialog helper...",
                    COLOR_BLUE
                );

                const auto overall_deadline =
                    std::chrono::steady_clock::now() + 30s;

                // -------------------------------------------------
                // Phase 1: Move + interact until dialog is detected.
                // -------------------------------------------------
                ShortDialogWatcher dialog(COLOR_YELLOW);
                bool dialog_started = false;

                while (!dialog_started) {

                    if (std::chrono::steady_clock::now() > overall_deadline) {
                        stream.log(
                            "[AutoStory] Clown helper timed out finding dialog.",
                            COLOR_RED
                        );
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "[AutoStory] Clown helper timed out finding dialog.", stream);
                    }

                    context.wait_for_all_requests();
                    pbf_press_dpad(context, direction, 80ms, 200ms);
                    pbf_press_button(context, BUTTON_A, 80ms, 500ms);

                    if (wait_until(stream, context, 1500ms, { {dialog} }) == 0) {
                        dialog_started = true;
                        stream.log("[AutoStory] Dialog detected.", COLOR_GREEN);
                    }
                }

                // -------------------------------------------------
                // Phase 2: A mash for 2 seconds.
                // -------------------------------------------------
                stream.log("[AutoStory] Mashing A to advance dialog...", COLOR_BLUE);
                pbf_mash_button(context, BUTTON_A, 4000ms);
                context.wait_for_all_requests();

                // -------------------------------------------------
                // Phase 3: Clear remaining dialog.
                // -------------------------------------------------
                pbf_mash_button(context, BUTTON_B, 4000ms);

                context.wait_for_all_requests();

                stream.log(
                    "[AutoStory] Clown dialog helper complete.",
                    COLOR_GREEN
                );
            }
            // ---------------------------------------------------------------------------
            // Time helper
            // ---------------------------------------------------------------------------

            // Options chrome and position detectors are independent of setting values.
            class TutorialOptionsDetector {
            public:
                bool detect(const ImageViewRGB32& screen) const {
                    ImageViewRGB32 panel = extract_box_reference(screen, ImageFloatBox{0.04, 0.065, 0.93, 0.70});
                    size_t white = 0;
                    for (size_t y = 0; y < panel.height(); y += 4)
                        for (size_t x = 0; x < panel.width(); x += 4) {
                            uint32_t p = panel.pixel(x, y);
                            if (((p >> 16) & 255) > 225 && ((p >> 8) & 255) > 225 && (p & 255) > 225) white++;
                        }
                    double samples = (double)(((panel.width() + 3) / 4) * ((panel.height() + 3) / 4));
                    // Distinguish Options from the icon menu and bag using its
                    // blue heading and full-width white settings panel.
                    ImageViewRGB32 heading = extract_box_reference(screen, ImageFloatBox{0.01, 0.005, 0.20, 0.025});
                    size_t blue = 0, total = 0;
                    for (size_t y = 0; y < heading.height(); y++)
                        for (size_t x = 0; x < heading.width(); x++) {
                            uint32_t p = heading.pixel(x, y);
                            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                            blue += b > r + 20 && g > r + 10;
                            total++;
                        }
                    return samples > 0 && white / samples > 0.70 && total > 0 && blue > total / 4;
                }
            };

            class TutorialOptionPositionDetector {
            public:
                // Locate the two long red horizontal lines surrounding the
                // selected row. Scrolling changes their position automatically.
                double selected_row(const ImageViewRGB32& screen) const {
                    int first = -1, last = -1;
                    for (size_t y = (size_t)(screen.height() * 0.08); y < screen.height() * 0.75; y++) {
                        size_t red = 0, total = 0;
                        for (size_t x = (size_t)(screen.width() * 0.08); x < screen.width() * 0.90; x += 2) {
                            uint32_t p = screen.pixel(x, y);
                            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                            red += r > 150 && r > g + 40 && r > b + 40;
                            total++;
                        }
                        if (total && red > total * 0.70) {
                            if (first < 0) first = (int)y;
                            last = (int)y;
                        }
                    }
                    if (first < 0 || last - first < screen.height() * 0.04 || last - first > screen.height() * 0.12) return -1;
                    return (first + last) / (2.0 * screen.height());
                }
                // Locate selected red value text separately from the row border.
                bool selected_value(const ImageViewRGB32& screen, double y) const {
                    ImageViewRGB32 value = extract_box_reference(screen, ImageFloatBox{0.33, y - 0.025, 0.54, 0.05});
                    size_t red = 0;
                    for (size_t py = 0; py < value.height(); py++)
                        for (size_t px = 0; px < value.width(); px++) {
                            uint32_t p = value.pixel(px, py);
                            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                            red += r > g + 20 && r > b + 20;
                        }
                    return red > value.width() * value.height() * 0.0005;
                }
            };


            // Compare binary visual shapes with small alignment tolerance.
            // No text recognition is needed for fixed English option labels.
            static double option_image_score(const ImageViewRGB32& screen,
                const ImageFloatBox& box, const ImageRGB32& reference, bool red_only) {
                ImageRGB32 actual = extract_box_reference(screen, box).scale_to(384, 32);
                ImageRGB32 expected = reference.scale_to(384, 32);
                auto ink = [red_only](uint32_t p){
                    int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                    return red_only ? r > g + 20 && r > b + 20 : r + g + b < 500;
                };
                double best = 0;
                for (int dy = -2; dy <= 2; dy++) for (int dx = -2; dx <= 2; dx++) {
                    size_t overlap = 0, count = 0;
                    for (int y = 2; y < 30; y++) for (int x = 2; x < 382; x++) {
                        bool a = ink(actual.pixel(x + dx, y + dy));
                        bool b = ink(expected.pixel(x, y));
                        overlap += a && b;
                        count += a + b;
                    }
                    if (count) best = std::max(best, 2.0 * overlap / count);
                }
                return best;
            }

            static bool autosave_off_highlight(const ImageViewRGB32& screen, double y) {
                auto red_count = [&](const ImageFloatBox& box){
                    ImageViewRGB32 image = extract_box_reference(screen, box);
                    size_t red = 0;
                    for (size_t py = 0; py < image.height(); py++)
                        for (size_t px = 0; px < image.width(); px++) {
                            uint32_t p = image.pixel(px, py);
                            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                            red += r > g + 30 && r > b + 30;
                        }
                    return red;
                };
                size_t off = red_count({0.63, y - 0.022, 0.055, 0.044});
                size_t on = red_count({0.34, y - 0.022, 0.055, 0.044});
                // Require a clear Off highlight and reject any competing On highlight.
                return off >= 10 && on * 10 < off;
            }

            static void verify_tutorial_settings_once(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                MenuDetector menu;
                VideoOverlaySet detector_overlays(stream.overlay());
                menu.make_overlays(detector_overlays);
                detector_overlays.add(COLOR_CYAN, {0.04, 0.065, 0.93, 0.70}, "");
                detector_overlays.add(COLOR_BLUE, {0.01, 0.005, 0.20, 0.025}, "");
                detector_overlays.add(COLOR_RED, {0.08, 0.08, 0.82, 0.67}, "");
                auto initial = stream.video().snapshot();
                if (!initial || !menu.detect(initial)) {
                    pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
                    context.wait_for_all_requests();
                }
                MenuWatcher menu_open;
                if (wait_until(stream, context, 5s, {{menu_open}}) != 0) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Settings checkpoint: Main menu not detected.", stream);
                }
                stream.log("[Tutorial settings] Main menu detected; settling for 500ms before Options navigation.");
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                TutorialOptionsDetector options_screen;
                TutorialOptionPositionDetector position;
                // Only two icons are available at this initial checkpoint.
                // Verify the main menu before navigating to the neighboring icon.
                bool options_open = false;
                for (size_t attempt = 0; attempt < 2; attempt++) {
                    pbf_press_dpad(context, attempt == 0 ? DPAD_RIGHT : DPAD_LEFT, 80ms, 350ms);
                    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                    context.wait_for_all_requests();
                    auto frame = stream.video().snapshot();
                    if (frame && options_screen.detect(frame)) {
                        options_open = true;
                        break;
                    }
                    pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                    context.wait_for_all_requests();
                    MenuWatcher back_to_menu;
                    if (wait_until(stream, context, 5s, {{back_to_menu}}) != 0) break;
                }
                if (!options_open) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Settings checkpoint: Options screen not detected.", stream);
                }
                stream.log("[Tutorial settings] Options screen detected.");
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                struct Setting { const char* label; const char* value; };
                const Setting settings[] = {
                    {"textspeed", "fast"}, {"changewindow", "type1"},
                    {"battleeffects", "off"}, {"battlestyle", "set"},
                    {"sendtoboxes", "automatic"}, {"givenicknames", "dontgive"},
                    {"gyroscope", "use"}, {"autosave", "off"},
                    {"shownicknames", "show"}, {"backgroundmusic", "8"},
                    {"soundeffects", "8"},
                };
                for (size_t row = 0; row < sizeof(settings) / sizeof(settings[0]); row++) {
                    // Detect the selected row afresh after every navigation/correction.
                    double y = -1;
                    const std::string base = RESOURCE_PATH() + "PokemonBDSP/Autostory/Options/" + settings[row].label;
                    const ImageRGB32 label_reference(base + "_label.png");
                    const ImageRGB32 value_reference(base + "_value.png");
                    bool slider_reset = false;
                    bool correct = false;
                    for (size_t attempt = 0; attempt < 30; attempt++) {
                        context.wait_for_all_requests();
                        auto frame = stream.video().snapshot();
                        if (frame && options_screen.detect(frame)) y = position.selected_row(frame);
                        else y = -1;
                        const bool value_visible = y >= 0 && position.selected_value(frame, y);
                        const double label_score = y >= 0 ? option_image_score(frame,
                            {0.08, y - 0.025, 0.24, 0.05}, label_reference, false) : 0;
                        stream.log("[Tutorial settings] Expected row=" + std::string(settings[row].label) +
                            " row_y=" + std::to_string(y) + " highlighted_value=" + std::to_string(value_visible) +
                            " label_match=" + std::to_string(label_score));
                        if (y < 0 || !value_visible || label_score < 0.80) {
                            // A menu can be detected while its row/text is still
                            // fading in. Re-read without directional inputs first.
                            if (attempt < 4) {
                                pbf_wait(context, 200ms);
                                context.wait_for_all_requests();
                                continue;
                            }
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Settings checkpoint: Unable to confirm expected row " + std::string(settings[row].label), stream);
                        }
                        // These are regular video-overlay boxes, so the existing
                        // overlay display control can show/hide every active ROI.
                        OverlayBoxScope row_overlay(stream.overlay(), COLOR_RED,
                            {0.07, y - 0.04, 0.84, 0.08}, "");
                        OverlayBoxScope label_overlay(stream.overlay(), COLOR_YELLOW,
                            {0.08, y - 0.025, 0.24, 0.05}, "");
                        OverlayBoxScope value_overlay(stream.overlay(), COLOR_GREEN,
                            {0.33, y - 0.025, 0.54, 0.05}, "");
                        VideoOverlaySet autosave_rois(stream.overlay());
                        if (row == 7) {
                            autosave_rois.add(COLOR_GREEN, {0.63, y - 0.022, 0.055, 0.044});
                            autosave_rois.add(COLOR_RED, {0.34, y - 0.022, 0.055, 0.044});
                        }
                        double score;
                        if (row >= 9) {
                            // Compare the numeric value rather than the long slider:
                            // neighboring slider levels otherwise look too similar.
                            ImageViewRGB32 number = value_reference.sub_image(
                                value_reference.width() * 82 / 100, 0,
                                value_reference.width() - value_reference.width() * 82 / 100,
                                value_reference.height());
                            ImageRGB32 number_reference = number.scale_to(96, 32);
                            score = option_image_score(frame,
                                {0.7728, y - 0.025, 0.0972, 0.05}, number_reference, true);
                        } else {
                            score = option_image_score(frame,
                                {0.33, y - 0.025, 0.54, 0.05}, value_reference, true);
                        }
                        stream.log("[Tutorial settings] " + std::string(settings[row].label) +
                            " target match=" + std::to_string(score));
                        // The reference recording uses inactive pale-red text;
                        // the focused live row has a thicker dark-red glyph mask.
                        // Verified Fast scores 0.734 while Slow/Normal score 0.
                        if (score >= 0.65 && (row != 7 || autosave_off_highlight(frame, y))) {
                            if (row == 7) {
                                for (size_t confirmation = 0; confirmation < 3; confirmation++) {
                                    pbf_wait(context, 200ms);
                                    context.wait_for_all_requests();
                                    auto check = stream.video().snapshot();
                                    double check_y = check ? position.selected_row(check) : -1;
                                    if (!check || !options_screen.detect(check) || check_y < 0 ||
                                        option_image_score(check, {0.08, check_y - 0.025, 0.24, 0.05}, label_reference, false) < 0.80 ||
                                        !autosave_off_highlight(check, check_y)) {
                                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                            "Autosave Off could not be confirmed. Stopping before movement.", stream);
                                    }
                                }
                            }
                            stream.log("[Tutorial settings] Verified " + std::string(settings[row].label) + "=" + settings[row].value);
                            correct = true;
                            break;
                        }
                        stream.log("[Tutorial settings] Correcting " + std::string(settings[row].label));
                        if (row >= 9 && !slider_reset) {
                            // Sliders clamp at zero. Establish a known endpoint,
                            // then increment; verify the target image afterward.
                            for (size_t step = 0; step < 12; step++)
                                pbf_press_dpad(context, DPAD_LEFT, 80ms, 100ms);
                            slider_reset = true;
                        } else {
                            pbf_press_dpad(context, DPAD_RIGHT, 80ms, 350ms);
                        }
                        context.wait_for_all_requests();
                    }
                    if (!correct) {
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Settings checkpoint: Could not apply " + std::string(settings[row].label), stream);
                    }
                    if (row + 1 < sizeof(settings) / sizeof(settings[0]))
                        pbf_press_dpad(context, DPAD_DOWN, 80ms, 350ms);
                }
                // Confirm settings before exiting: A A A A B B.
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1200ms);
                pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                context.wait_for_all_requests();
                OverworldWatcher overworld;
                if (wait_until(stream, context, 10s, {{overworld}}) != 0) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Settings checkpoint: Did not return to the overworld after saving options.", stream);
                }
                // Reopen the saved Options screen and independently verify Autosave.
                // No route movement is permitted until this persisted-value check passes.
                pbf_press_button(context, BUTTON_X, 80ms, 1000ms);
                context.wait_for_all_requests();
                MenuWatcher verify_menu;
                if (wait_until(stream, context, 5s, {{verify_menu}}) != 0) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Autosave verification: Main menu not detected.", stream);
                }
                bool reopened = false;
                for (size_t attempt = 0; attempt < 2; attempt++) {
                    pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                    context.wait_for_all_requests();
                    auto check = stream.video().snapshot();
                    if (check && options_screen.detect(check)) { reopened = true; break; }
                    pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                    context.wait_for_all_requests();
                    MenuWatcher return_menu;
                    if (wait_until(stream, context, 5s, {{return_menu}}) != 0) break;
                    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 350ms);
                }
                if (!reopened) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Autosave verification: Could not reopen saved Options.", stream);
                }
                // Options starts on Text Speed; Autosave is seven rows below it.
                for (size_t step = 0; step < 7; step++)
                    pbf_press_dpad(context, DPAD_DOWN, 80ms, 350ms);
                context.wait_for_all_requests();
                const ImageRGB32 autosave_label(RESOURCE_PATH() + "PokemonBDSP/Autostory/Options/autosave_label.png");
                for (size_t confirmation = 0; confirmation < 3; confirmation++) {
                    auto check = stream.video().snapshot();
                    double check_y = check ? position.selected_row(check) : -1;
                    if (!check || !options_screen.detect(check) || check_y < 0 ||
                        option_image_score(check, {0.08, check_y - 0.025, 0.24, 0.05}, autosave_label, false) < 0.80 ||
                        !autosave_off_highlight(check, check_y)) {
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Saved Autosave setting is not verified Off. Stopping before movement.", stream);
                    }
                    pbf_wait(context, 200ms);
                    context.wait_for_all_requests();
                }
                stream.log("[Tutorial settings] Autosave = Off: confirmed after reopening saved Options (3 frames).");
                // Confirm settings before exiting: A A A A B B.
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_A, 80ms, 1200ms);
                pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
                context.wait_for_all_requests();
                OverworldWatcher verified_exit;
                if (wait_until(stream, context, 10s, {{verified_exit}}) != 0) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Autosave verification: Could not return to the overworld.", stream);
                }
                // Report inferred values explicitly: these are visual matches,
                // allowing the user to compare our conclusions with the screen.
                stream.log("[Tutorial settings] Completed. Values believed applied (visual matches before saving):");
                stream.log("[Tutorial settings] Text Speed = Fast; Change Window = Type 1; Battle Effects = Off; Battle Style = Set.");
                stream.log("[Tutorial settings] Send to Boxes = Automatic; Give Nicknames = Don't give; Gyroscope = Use.");
                stream.log("[Tutorial settings] Autosave = Off; Show Nicknames = Show; Background Music = 8; Sound Effects = 8.");
            }


            void verify_tutorial_settings(VideoStream& stream, ProControllerContext& context) {
                // Settings-only recovery: initial attempt plus three retries.
                // Never permit route movement unless the complete check succeeds.
                constexpr size_t MAX_RETRIES = 3;
                for (size_t attempt = 0; attempt <= MAX_RETRIES; attempt++) {
                    stream.log("[Tutorial settings] Attempt " + std::to_string(attempt + 1) + "/4.");
                    try {
                        verify_tutorial_settings_once(stream, context);
                        return;
                    } catch (OperationFailedException& error) {
                        stream.log("[Tutorial settings] Attempt failed: " + error.message(), COLOR_ORANGE);
                        if (attempt == MAX_RETRIES) {
                            stream.log("[Tutorial settings] All three retries exhausted; stopping before movement.", COLOR_RED);
                            throw;
                        }
                        // Back out of Options, save prompts, bag, or menu without
                        // moving the player or resetting/loading the game.
                        stream.log("[Tutorial settings] Returning to overworld before retry.");
                        context.wait_for_all_requests();
                        TutorialOptionsDetector options_screen;
                        const auto failed_frame = stream.video().snapshot();
                        if (failed_frame && options_screen.detect(failed_frame)) {
                            // Finish the known Options save/exit sequence before
                            // observing state; never cancel it on an Options frame
                            // misclassified as overworld by the generic detector.
                            for (size_t press = 0; press < 4; press++)
                                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                        }
                        pbf_mash_button(context, BUTTON_B, 3000ms);
                        context.wait_for_all_requests();
                        OverworldDetector overworld;
                        MenuDetector menu;
                        BlackScreenDetector black;
                        bool recovered = false;
                        size_t confirmations = 0;
                        for (size_t check = 0; check < 15; check++) {
                            const auto frame = stream.video().snapshot();
                            const bool clear = frame && !options_screen.detect(frame) &&
                                !menu.detect(frame) && !black.detect(frame) && bag_pocket_position(frame) == 0 && overworld.detect(frame);
                            if (clear) {
                                if (++confirmations >= 3) { recovered = true; break; }
                                pbf_wait(context, 200ms);
                            } else {
                                confirmations = 0;
                                pbf_mash_button(context, BUTTON_B, 500ms);
                            }
                            context.wait_for_all_requests();
                        }
                        stream.log(recovered ? "[Tutorial settings] Recovery confirmed outside Options/menu; retrying."
                            : "[Tutorial settings] Recovery not confirmed; next attempt will recheck the screen.");
                        pbf_wait(context, 500ms);
                        context.wait_for_all_requests();
                    }
                }
            }

            void set_time_fast(
                SingleSwitchProgramEnvironment& env,
                ProControllerContext& context,
                const DateTime& target,
                bool preserve_date
            ) {
                env.log("[AutoStory] Fast date set starting...", COLOR_ORANGE);

                // 1) Home
                NintendoSwitch::go_home(env.console, context);

                // 2) Navigate to Date/Time settings quickly.
                NintendoSwitch::home_to_date_time(env.console, context, true);

                // 3) Enter date editor.
                pbf_press_button(context, BUTTON_A, 80ms, 240ms);

                // 4) Wait for date screen and set exact time.
                NintendoSwitch::DateChangeWatcher date_reader(env.console);
                int ret = wait_until(
                    env.console, context, 10s,
                    { date_reader }
                );
                if (ret < 0) {
                    OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::SEND_ERROR_REPORT,
                        "Failed to enter Date Change screen.",
                        env.console
                    );
                }

                {
                    VideoOverlaySet overlays(env.console.overlay());
                    date_reader.make_overlays(overlays);
                    DateTime effective_target = target;
                    if (preserve_date) {
                        context.wait_for_all_requests();
                        const auto snapshot = env.console.video().snapshot();
                        if (!snapshot) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Clock-only setup: No date-editor frame.", env.console);
                        const DateTime current = date_reader.read_date(env.console, snapshot.frame).second;
                        if (current.year < 2000 || current.year > 2060 || current.month < 1 || current.month > 12 ||
                            current.day < 1 || current.day > 31)
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Clock-only setup: Could not reliably read the current console date.", env.console);
                        effective_target.year = current.year;
                        effective_target.month = current.month;
                        effective_target.day = current.day;
                        env.log("[AutoStory] Clock-only target=" + std::to_string((int)target.hour) + ":" +
                            std::to_string((int)target.minute) + "; preserving date=" + std::to_string(current.year) + "-" +
                            std::to_string((int)current.month) + "-" + std::to_string((int)current.day) + ".");
                    }
                    date_reader.set_date(env.program_info(), env.console, context, effective_target);
                }

                // 5) Commit date.
                pbf_press_button(context, BUTTON_A, 160ms, 240ms);

                // 6) Back to game.
                pbf_press_button(
                    context, BUTTON_HOME, 160ms,
                    ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0
                );
                NintendoSwitch::resume_game_from_home(env.console, context, false);

                context.wait_for_all_requests();
                context.wait_for(500ms);
            }

         
            // ---------------------------------------------------------------------------
            // Natalie battle
            // ---------------------------------------------------------------------------
            static void select_battle_move_slot(VideoStream& stream, ProControllerContext& context,
                BattleType battle_type, size_t target, const std::string& label,
                bool confirm_default_target = false, size_t current_move = 1) {
                BattleMenuDetector commands(battle_type);
                VideoOverlaySet overlays(stream.overlay());
                commands.make_overlays(overlays);
                overlays.add(COLOR_GREEN, {0.90, 0.48, 0.075, 0.505});
                // Reenter Fight from an already-open list without assuming
                // that this resets the remembered move cursor.
                if (confirm_default_target) {
                    context.wait_for_all_requests();
                    const auto frame=stream.video().snapshot();
                    if (frame && detect_battle_move_menu(frame).move_count) {
                        pbf_press_button(context,BUTTON_B,80ms,500ms);
                    }
                }
                bool fight = false;
                size_t confirmed = 0;
                for (size_t check = 0; check < 30; check++) {
                    context.wait_for_all_requests();
                    auto frame = stream.video().snapshot();
                    BattleMenuOption selected = frame ? commands.selected_option(frame) : BattleMenuOption::UNKNOWN;
                    stream.log("[" + label + "] Command cursor=" + std::to_string((int)selected + 1) + "; target Battle=1.");
                    if (selected == BattleMenuOption::BATTLE) {
                        if (++confirmed >= 3) { fight = true; break; }
                        pbf_wait(context, 100ms);
                    } else {
                        confirmed = 0;
                        if (selected == BattleMenuOption::UNKNOWN) pbf_wait(context, 150ms);
                        else pbf_press_dpad(context, DPAD_UP, 80ms, 250ms);
                    }
                }
                if (!fight) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label + ": Could not confirm Battle command; A not pressed.", stream);
                pbf_press_button(context,BUTTON_A,80ms,confirm_default_target ? 1000ms : 500ms);
                if (confirm_default_target) {
                    // Reuse the farming move/target confirmation sequence, but
                    // navigate relative to this battle's retained move cursor.
                    // Reopening Fight does not reset it to move 1.
                    size_t available=0,stable=0;
                    for (size_t check=0;check<20 && stable<3;check++) {
                        context.wait_for_all_requests();
                        const auto frame=stream.video().snapshot();
                        const auto moves=frame ? detect_battle_move_menu(frame) : BattleMoveMenuState{};
                        if (moves.move_count && moves.move_count==available) stable++;
                        else {available=moves.move_count;stable=moves.move_count ? 1:0;}
                        if (stable<3) pbf_wait(context,100ms);
                    }
                    if (stable<3 || available<target || available<current_move) OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::SEND_ERROR_REPORT,label+": Requested move position is not available.",stream);
                    stream.log("["+label+"] Retained move cursor "+std::to_string(current_move)+
                        " -> "+std::to_string(target)+".");
                    for (size_t slot=current_move;slot<target;slot++) pbf_press_dpad(context,DPAD_DOWN,80ms,400ms);
                    for (size_t slot=current_move;slot>target;slot--) pbf_press_dpad(context,DPAD_UP,80ms,400ms);
                    pbf_press_button(context,BUTTON_A,80ms,1000ms);
                    pbf_press_button(context,BUTTON_A,80ms,3000ms);
                    return;
                }
                confirmed = 0;
                size_t previous_move_count = 0, stable_move_count = 0;
                for (size_t check = 0; check < 30; check++) {
                    context.wait_for_all_requests();
                    auto frame = stream.video().snapshot();
                    const BattleMoveMenuState moves = frame ? detect_battle_move_menu(frame) : BattleMoveMenuState{};
                    const int slot = (int)moves.selected_slot;
                    VideoOverlaySet move_overlays(stream.overlay());
                    for (size_t i = 0; i < moves.move_count; i++)
                        move_overlays.add(moves.selected_slot == i + 1 ? COLOR_GREEN : COLOR_YELLOW, moves.buttons[i]);
                    stream.log("[" + label + "] Available moves=" + std::to_string(moves.move_count) +
                        "; selected slot=" + std::to_string(slot) + "/4; target=" + std::to_string(target) + ".");
                    if (moves.move_count == previous_move_count && moves.move_count > 0) stable_move_count++;
                    else { previous_move_count = moves.move_count; stable_move_count = 1; confirmed = 0; }
                    if (moves.move_count > 0 && moves.move_count < target && stable_move_count >= 3) {
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label + ": Move slot " + std::to_string(target) + " is absent; no move confirmed.", stream);
                    }
                    if (slot == (int)target && stable_move_count >= 3) {
                        if (++confirmed >= 3) {
                            stream.log("[" + label + "] Move " + std::to_string(target) + " confirmed; using it.");
                            pbf_press_button(context, BUTTON_A, 80ms, 3000ms);
                            return;
                        }
                        pbf_wait(context, 100ms);
                    } else {
                        confirmed = 0;
                        if (slot == 0 || slot == (int)target) pbf_wait(context, 150ms);
                        else pbf_press_dpad(context, slot < (int)target ? DPAD_DOWN : DPAD_UP, 80ms, 250ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label + ": Could not confirm move " + std::to_string(target) + "; A not pressed.", stream);
            }

            static void select_natalie_move_three(VideoStream& stream, ProControllerContext& context) {
                select_battle_move_slot(stream, context, BattleType::TRAINER, 3, "Natalie");
            }

            void fight_Natalie(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log(
                    "[AutoStory] Starting Natalie battle...",
                    COLOR_BLUE
                );

                // Allow battle transition to finish.
                pbf_wait(context, 5000ms);

                bool battle_menu_seen = false;

                // State machine loop.
                while (true) {

                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(BattleType::TRAINER);
                    EndBattleWatcher end_battle;

                    // Detect move-learning arrows.
                    SelectionArrowFinder learn_move(
                        stream.overlay(),
                        {0.50, 0.62, 0.40, 0.18},
                        COLOR_YELLOW
                    );

                    int ret = run_until<ProControllerContext>(
                        stream,
                        context,
                        [](ProControllerContext& context) {
                            // Continuously clear:
                            // EXP gain
                            // level up
                            // faint text
                            // send-out text
                            // battle narration
                            pbf_mash_button(
                                context,
                                BUTTON_B,
                                120000ms
                            );
                        },
                        {
                            {battle_menu},
                            battle_menu_seen
                                ? PeriodicInferenceCallback{end_battle}
                                : PeriodicInferenceCallback{},
                            {learn_move},
                        }
                    );

                    switch (ret) {

                        // -------------------------------------------------
                        // Battle menu detected.
                        // -------------------------------------------------
                    case 0:
                    {
                        stream.log(
                            "[AutoStory] Battle menu detected.",
                            COLOR_BLUE
                        );

                        battle_menu_seen = true;

                        select_natalie_move_three(stream, context);

                        break;
                    }

                    // -------------------------------------------------
                    // Battle finished.
                    // -------------------------------------------------
                    case 1:
                    {
                        stream.log(
                            "[AutoStory] Natalie battle complete.",
                            COLOR_GREEN
                        );

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        return;
                    }

                    // -------------------------------------------------
                    // Move learning detected.
                    // -------------------------------------------------
                    case 2:
                    {
                        stream.log(
                            "[AutoStory] Learn move detected.",
                            COLOR_ORANGE
                        );

                        // Always refuse move learning.
                        pbf_move_right_joystick(
                            context,
                            { 0, -1 },
                            160ms,
                            840ms
                        );

                        pbf_press_button(
                            context,
                            BUTTON_A,
                            160ms,
                            840ms
                        );

                        break;
                    }

                    // -------------------------------------------------
                    // Timeout.
                    // -------------------------------------------------
                    default:
                    {
                        stream.log(
                            "[AutoStory] Natalie battle timed out.",
                            COLOR_RED
                        );

                        pbf_mash_button(
                            context,
                            BUTTON_B,
                            5000ms
                        );

                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "[AutoStory] Natalie battle timed out.", stream);
                    }

                    }
                }
            }

            // ---------------------------------------------------------------------------
            // Starly battle
            // ---------------------------------------------------------------------------

            void fight_starly(VideoStream& stream, ProControllerContext& context) {
                stream.log("[AutoStory] Starting Starly battle...", COLOR_BLUE);

                // Declare OUTSIDE the loop so accumulated detector state
                // (trigger counts, black-screen transitions) is preserved across frames.
                // STARTER type: player has no Pokeballs, so ball-count areas won't
                // be white. STARTER skips that check and only verifies status bars
                // and the Fight button color.
                BattleMenuWatcher battle_menu(BattleType::STARTER, COLOR_YELLOW);
                EndBattleWatcher  end_battle;

                while (true) {
                    context.wait_for_all_requests();

                    // Phase 1: wait for either battle menu or battle end.
                    int ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx) {
                            pbf_mash_button(ctx, BUTTON_B, 120000ms);
                        },
            {
                {battle_menu},
                {end_battle},
            }
            );

                    switch (ret) {
                    case 0:
                    {
                        stream.log("[AutoStory] Battle menu detected. Selecting move...", COLOR_BLUE);

                        // Select Fight, then move slot 1.
                        pbf_press_button(context, BUTTON_A, 80ms, 500ms);
                        pbf_press_button(context, BUTTON_A, 80ms, 3000ms);
                        // Loop back - mash B through animations until next menu or end.
                        break;
                    }
                    case 1:
                    {
                        stream.log("[AutoStory] Starly battle complete.", COLOR_GREEN);

                        // Mash through XP / level-up / dialog screens back to overworld.
                        OverworldWatcher overworld;
                        int cleanup_ret = run_until<ProControllerContext>(
                            stream, context,
                            [](ProControllerContext& ctx) {
                                pbf_mash_button(ctx, BUTTON_B, 120000ms);
                            },
                            { {overworld} }
                        );
                        if (cleanup_ret != 0) {
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                "Starly battle: Overworld cleanup timed out.", stream);
                        }
                        return;
                    }
                    default:
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Starly battle timed out.", stream);
                    }
                }
            }

            // ---------------------------------------------------------------------------
            // Tristan battle
            // ---------------------------------------------------------------------------
            static void encounter_one_wild_pokemon(VideoStream& stream, ProControllerContext& context) {
                context.wait_for_all_requests();
                pbf_wait(context, 1000ms);

                bool encounter_found = false;
                const auto deadline = std::chrono::steady_clock::now() + 180s;

                // Wiggle left/right until battle starts.
                while (!encounter_found && std::chrono::steady_clock::now() < deadline) {
                    BlackScreenWatcher battle_start;
                    BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
                    BattleDialogWatcher battle_dialog(COLOR_YELLOW);

                    // Turn left.
                    pbf_move_left_joystick(context, {-1, 0}, 120ms, 200ms);
                    context.wait_for_all_requests();

                    int ret = wait_until(
                        stream,
                        context,
                        1500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (ret >= 0) {
                        encounter_found = true;
                        break;
                    }

                    // Turn right.
                    pbf_move_left_joystick(context, {1, 0}, 120ms, 200ms);
                    context.wait_for_all_requests();

                    ret = wait_until(
                        stream,
                        context,
                        1500ms,
                        {
                            {battle_start},
                            {battle_menu},
                            {battle_dialog},
                        }
                    );

                    if (ret >= 0) {
                        encounter_found = true;
                        break;
                    }
                }

                if (!encounter_found) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Wild encounter: Left/right wiggle did not find a battle within three minutes.", stream);
                stream.log("[AutoStory] Wild battle detected!", COLOR_GREEN);

                // Wait until the actual battle menu appears.
                BattleMenuWatcher confirm_menu(BattleType::STANDARD);

                int menu_ret = wait_until(
                    stream,
                    context,
                    15s,
                    {
                        {confirm_menu},
                    }
                );

                if (menu_ret < 0) {
                    stream.log(
                        "[AutoStory] Failed to detect battle menu.",
                        COLOR_RED
                    );
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "[AutoStory] Failed to detect battle menu.", stream);
                }

                stream.log("[AutoStory] Battle menu detected.", COLOR_GREEN);

                context.wait_for_all_requests();
                pbf_wait(context, 500ms);

            }

            static void confirm_battle_overworld(VideoStream& stream, ProControllerContext& context,
                const std::string& label) {
                OverworldDetector overworld;
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BlackScreenDetector black;
                VideoOverlaySet overlays(stream.overlay());
                overworld.make_overlays(overlays);
                dialogue.make_overlays(overlays);
                black.make_overlays(overlays);
                const auto deadline = std::chrono::steady_clock::now() + 180s;
                auto quiet_since = std::chrono::steady_clock::now();
                bool finished = false;
                while (std::chrono::steady_clock::now() < deadline) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label + ": No video frame.", stream);
                    const auto now = std::chrono::steady_clock::now();
                    if (dialogue.detect(frame) || black.detect(frame) || !overworld.detect(frame)) {
                        quiet_since = now;
                    } else if (now - quiet_since >= 4s) {
                        finished = true;
                        break;
                    }
                    pbf_mash_button(context, BUTTON_B, 100ms);
                    context.wait_for_all_requests();
                }
                if (!finished) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label + ": Dialogue-free overworld not confirmed for four seconds.", stream);
                context.wait_for_all_requests();
            }

            static void fight_using_move_list(
                VideoStream& stream, ProControllerContext& context,
                BattleType battle_type, const std::vector<size_t>& move_slots,
                const std::string& label, bool confirm_default_target, bool post_battle_dialogue_cleanup
            ) {
                if (move_slots.empty() || std::any_of(move_slots.begin(),move_slots.end(),
                    [](size_t slot) { return slot<1 || slot>4; })) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label+": Supply a nonempty move list containing only positions 1 through 4.",stream);
                }
                size_t move_index=0;
                // The first Fight menu starts at move 1. Later turns retain
                // this Pokemon's previous selection; a new battle resets it.
                size_t current_move=1;
                stream.log(
                    "[AutoStory] Starting " + label + " battle...",
                    COLOR_BLUE
                );

                // Scripted battles wait on the menu detector below, so no
                // extra fixed delay is needed after a caller confirms it.
                // Preserve the existing early-game transition delay.
                if (!confirm_default_target) pbf_wait(context,5000ms);

                bool battle_menu_seen = false;
                EndBattleWatcher end_battle;

                while (true) {

                    context.wait_for_all_requests();

                    BattleMenuWatcher battle_menu(battle_type);
                    // Reuse the farming handlers' move-learning prompt recovery.
                    SelectionArrowFinder learn_move(stream.overlay(),{0.50,0.62,0.40,0.18},COLOR_YELLOW);

                    int ret = run_until<ProControllerContext>(
                        stream,
                        context,
                        [](ProControllerContext& context) {
                            // Clear narration with B while watching for commands.
                            // A here can open Fight before selection begins.
                            pbf_mash_button(context,BUTTON_B,120000ms);
                        },
                        {
                            {battle_menu},
                            battle_menu_seen
                                ? PeriodicInferenceCallback{end_battle}
                                : PeriodicInferenceCallback{},
                            confirm_default_target ? PeriodicInferenceCallback{learn_move} : PeriodicInferenceCallback{},
                        }
                    );

                    switch (ret) {

                        // -------------------------------------------------
                        // Battle menu detected.
                        // -------------------------------------------------
                    case 0:
                        battle_menu_seen = true;

                        stream.log(
                            "[AutoStory] Battle menu detected.",
                            COLOR_BLUE
                        );

                        stream.log("["+label+"] Move list position "+std::to_string(move_index+1)+
                            ": use move "+std::to_string(move_slots[move_index])+".");
                        select_battle_move_slot(stream, context, battle_type, move_slots[move_index], label,
                            confirm_default_target,current_move);
                        current_move=move_slots[move_index];
                        // Advance once per move selection; retain the final entry.
                        if (move_index+1<move_slots.size()) move_index++;

                        break;

                        // -------------------------------------------------
                        // End battle detected.
                        // -------------------------------------------------
                    case 1: {
                        stream.log(
                            "[AutoStory] " + label + " battle complete.",
                            COLOR_GREEN
                        );

                        // EndBattleWatcher has confirmed the completed exit transition.
                        // Ordinary trainers need no scene-wide dialogue-free gate.
                        if (!post_battle_dialogue_cleanup) {
                            context.wait_for_all_requests();
                            return;
                        }
                        // Story NPCs and gym leaders opt into the existing cleanup.
                        OverworldWatcher overworld;
                        int cleanup_ret = run_until<ProControllerContext>(stream, context,
                            [](ProControllerContext& ctx) {
                                pbf_mash_button(ctx, BUTTON_B, 120000ms);
                            }, {{overworld}});
                        if (cleanup_ret != 0) {
                            OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                label + ": Post-battle cleanup timed out.", stream);
                        }
                        context.wait_for_all_requests();
                        confirm_battle_overworld(stream, context, label + " cleanup");
                        return;
                    }

                    case 2:
                        stream.log("["+label+"] Declining move learning; preserving scripted move positions.");
                        pbf_move_right_joystick(context,{0,-1},160ms,840ms);
                        pbf_press_button(context,BUTTON_ZL,160ms,840ms);
                        break;

                    default:
                        stream.log(
                            "[AutoStory] " + label + " battle timeout.",
                            COLOR_RED
                        );

                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "[AutoStory] " + label + " battle timeout.", stream);
                    }
                }
            }

            // Preserve the existing early-game callers and their B text cleanup.
            static void fight_using_move_slot(VideoStream& stream, ProControllerContext& context,
                BattleType battle_type, size_t move_slot, const std::string& label) {
                fight_using_move_list(stream,context,battle_type,{move_slot},label,false,true);
            }

            void clear_post_battle_dialogue(VideoStream& stream, ProControllerContext& context,
                Button button, Milliseconds quiet_period,
                const char* instruction_card) {
                context.wait_for_all_requests();
                std::unique_ptr<ImageMatchDetector> card;
                if (instruction_card) card = std::make_unique<ImageMatchDetector>(
                    std::make_shared<ImageRGB32>(RESOURCE_PATH()+instruction_card),
                    ImageFloatBox{.04,.32,.92,.34},30.0);
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BlackScreenDetector black;
                VideoOverlaySet overlays(stream.overlay());
                dialogue.make_overlays(overlays);
                black.make_overlays(overlays);
                const auto deadline = std::chrono::steady_clock::now() + 120s;
                auto absent_since = std::chrono::steady_clock::now();
                while (std::chrono::steady_clock::now() < deadline) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Post-battle dialogue: No video frame.", stream);
                    const auto now = std::chrono::steady_clock::now();
                    const bool dialogue_visible = dialogue.detect(*frame.frame);
                    const bool card_visible = card && card->detect(*frame.frame);
                    if (dialogue_visible || card_visible || black.detect(*frame.frame)) absent_since = now;
                    else if (now - absent_since >= quiet_period) {
                        stream.log("[Post-battle dialogue] " + std::to_string(quiet_period.count()) +
                            "ms without dialogue confirmed.");
                        return;
                    }
                    // A can interact again once dialogue disappears. Only confirm
                    // visible prompts; B retains the existing post-battle behavior.
                    if (card_visible) pbf_press_button(context, BUTTON_B, 80ms, 100ms);
                    else if (button == BUTTON_A && !dialogue_visible) pbf_wait(context, 100ms);
                    else pbf_mash_button(context, button, 100ms);
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    "Post-battle dialogue: Three clear seconds not confirmed.", stream);
            }

            void align_to_overworld_landmarks(VideoStream& stream, ProControllerContext& context,
                OverworldMatchResult (*matcher)(const ImageRGB32&), const std::string& label, bool allow_story_start, bool horizontal_first,
                bool search_down_first,
                OverworldMatchResult (*entry_matcher)(const ImageRGB32&),
                bool* entry_detected,
                double arrival_tolerance_x, double arrival_tolerance_y,
                double moderate_entry_confidence, bool* rock_acquisition_timed_out) {
                if (rock_acquisition_timed_out) *rock_acquisition_timed_out=false;
                auto missing_rocks_since=std::chrono::steady_clock::time_point::max();
                if (entry_detected) *entry_detected=false;
                context.wait_for_all_requests();
                pbf_wait(context, 500ms);
                context.wait_for_all_requests();
                ShortDialogDetector dialogue(COLOR_YELLOW);
                BattleMenuDetector battle(BattleType::TRAINER, COLOR_GREEN);
                size_t confirmations = 0, missing = 0;
                for (size_t attempt = 0; attempt < 80; attempt++) {
                    const auto frame = stream.video().snapshot();
                    if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        label + ": No video frame.", stream);
                    if (entry_matcher){
                        BlackScreenDetector transition;
                        if (entry_matcher(*frame.frame).located || transition.detect(*frame.frame)){
                            BlackScreenWatcher visible(COLOR_RED,{0.1,0.1,0.8,0.8},
                                100,10,BlackScreenWatcher::FinderType::GONE,500ms);
                            if (wait_until(stream,context,30s,{{visible}})!=0)
                                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                    label+": Building transition did not finish.",stream);
                            const auto inside=stream.video().snapshot();
                            if (!inside || !entry_matcher(*inside.frame).located)
                                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                    label+": Expected interior not confirmed after transition.",stream);
                            if(entry_detected)*entry_detected=true;
                            stream.log("["+label+"] Expected interior confirmed; ending outdoor alignment.",COLOR_GREEN);
                            return;
                        }
                    }
                    // The story can start before the exact reference point. Hand off
                    // to battle preparation instead of steering during dialogue.
                    if (dialogue.detect(*frame.frame) || battle.detect(*frame.frame)) {
                        if (allow_story_start) {
                            stream.log("[" + label + "] Story dialogue/battle started; handing off without further movement.");
                            return;
                        }
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label + ": Unexpected dialogue/battle during alignment.", stream);
                    }
                    const auto result = matcher(*frame.frame);
                    if (result.located)
                        missing_rocks_since=std::chrono::steady_clock::time_point::max();
                    VideoOverlaySet overlays(stream.overlay());
                    for (const auto& landmark : result.landmarks) {
                        overlays.add(COLOR_CYAN, landmark.search);
                        if (landmark.accepted) overlays.add(COLOR_GREEN, landmark.found);
                        stream.log("[" + label + "] " + landmark.name + " score=" +
                            std::to_string(landmark.score) + ", margin=" + std::to_string(landmark.margin) +
                            ", offset=(" + std::to_string(landmark.dx) + "," + std::to_string(landmark.dy) + ")");
                    }
                    stream.log("[" + label + "] located=" + std::to_string(result.located) +
                        ", coarse=" + std::to_string(result.coarse) + ", at_target=" + std::to_string(result.at_target) +
                        ", consensus offset=(" + std::to_string(result.dx) + "," + std::to_string(result.dy) + ")");
                    const double entry_confidence=moderate_entry_confidence>0 ? overworld_entry_confidence(result) : 0;
                    const bool moderate_arrival=moderate_entry_confidence>0 && entry_confidence>=moderate_entry_confidence;
                    if(moderate_entry_confidence>0) stream.log("["+label+"] Exterior entry confidence="+
                        std::to_string(entry_confidence)+", moderate threshold="+std::to_string(moderate_entry_confidence)+
                        ", strict target="+std::to_string(result.at_target));
                    // Explicit loose bounds are for non-interacting route handoffs.
                    // Default callers still require the matcher's precise arrival.
                    const bool rough_arrival=arrival_tolerance_x>0 && arrival_tolerance_y>0 &&
                        result.located && std::abs(result.dx)<=arrival_tolerance_x &&
                        std::abs(result.dy)<=arrival_tolerance_y;
                    if (result.at_target || rough_arrival || moderate_arrival) {
                        if (++confirmations >= 3) { stream.log("[" + label + "] Position confirmed."); return; }
                        pbf_wait(context, 250ms);
                    } else if (result.located) {
                        confirmations = 0;
                        missing = 0;
                        // Wait at an aligned pose while strict visual support
                        // recovers, rather than stepping away under a cloud.
                        if ((matcher==match_galactic_hq_jupiter ||
                            matcher==match_route208_hannah_departure || matcher==match_route208_stair_top) && result.coarse &&
                            std::abs(result.dx)<=1.0/128 && std::abs(result.dy)<=1.0/54){
                            pbf_wait(context,500ms);
                            context.wait_for_all_requests();
                            continue;
                        }
                        // A coarse match cannot confirm position, but two accepted
                        // anchors can establish a safe horizontal direction when both
                        // offsets are well outside the uncertainty in their spacing.
                        bool coarse_horizontal = false;
                        if (horizontal_first && result.coarse && result.landmarks.size() == 2) {
                            const auto& a = result.landmarks[0];
                            const auto& b = result.landmarks[1];
                            const double nearest = std::min(std::abs(a.dx), std::abs(b.dx));
                            coarse_horizontal = a.accepted && b.accepted && a.dx*b.dx > 0 &&
                                nearest > 1.0/64 && nearest > 2*std::abs(a.dx-b.dx);
                        }
                        const bool plaza_anchor_horizontal = matcher == match_jubilife_plaza_approach &&
                            result.coarse && std::abs(result.dx)>1.0/50 && std::abs(result.dy)<=1.0/54;
                        // A uniquely matched Eterna ball may correct the door
                        // column while the sign/bush enter the frame. Arrival
                        // still requires the matcher's full three-anchor check.
                        const bool eterna_anchor_horizontal = (matcher == match_eterna_center_entrance ||
                            matcher == match_eterna_underground_house || matcher == match_eterna_underground_man) &&
                            result.coarse && std::abs(result.dx)>1.0/128;
                        // Recover the gym's doorway row before moving right if
                        // the south approach overshot toward the fence/cliff.
                        const bool gym_anchor_horizontal=matcher==match_eterna_gym_entrance &&
                            result.coarse && std::abs(result.dx)>1.0/128 && std::abs(result.dy)<=.06;
                        const bool gym_row_recovery=matcher==match_eterna_gym_entrance &&
                            result.coarse && std::abs(result.dy)>.06;
                        // Reach the row north of Jupiter before entering her
                        // column. From above that row, align sideways before
                        // moving south so the NPC cannot block the correction.
                        const bool jupiter_row_recovery=matcher==match_galactic_hq_jupiter &&
                            result.dy < -1.0/54;
                        const bool jupiter_anchor_horizontal=matcher==match_galactic_hq_jupiter &&
                            !jupiter_row_recovery && result.coarse && std::abs(result.dx)>1.0/128;
                        // Stay south of the Contest Hall wall/sign before crossing to Fantina.
                        const bool contest_row_recovery=matcher==match_hearthome_contest_fantina && result.dy>1.0/54;
                        // Keep the stair approach horizontal-first until close to
                        // its column. Recorded fine poses otherwise spent repeated
                        // 16ms sideways turns while a larger vertical error waited.
                        // Fine correction changes axis priority, never arrival gates.
                        const bool stair_fine_correction=matcher==match_route208_stair_top &&
                            !result.coarse && std::abs(result.dx)<=1.0/64;
                        const bool horizontal = !contest_row_recovery && !gym_row_recovery && !jupiter_row_recovery && (jupiter_anchor_horizontal || coarse_horizontal || plaza_anchor_horizontal || eterna_anchor_horizontal || gym_anchor_horizontal || (!result.coarse &&
                            ((horizontal_first && !stair_fine_correction && std::abs(result.dx)>1.0/128) ||
                             std::abs(result.dx)>=std::abs(result.dy)*9.0/16)));
                        if (stair_fine_correction && horizontal_first && !horizontal && std::abs(result.dx)>1.0/128)
                            stream.log("["+label+"] Near stair column; correcting the larger vertical error first.");
                        if (coarse_horizontal) stream.log("[" + label +
                            "] Both landmarks agree on a large horizontal offset; correcting the street lane first.");
                        const double error = horizontal ? result.dx : result.dy*9.0/16;
                        auto duration = std::abs(error) > 1.0/24 ? 80ms : std::abs(error) > 1.0/64 ? 40ms : 16ms;
                        // A 16ms turn may not complete a step. Use a bounded
                        // full correction while reacquiring the second plaza anchor.
                        if (matcher == match_jubilife_plaza_approach && result.coarse)
                            duration = std::max(duration, 60ms);
                        if (matcher==match_galactic_hq_jupiter){
                            for (const auto& landmark:result.landmarks){
                                // The player stands around y=.74. Do not steer
                                // sideways/north when the stair opening is close
                                // to that row; a southward correction moves away.
                                if (landmark.name=="StairBoundary" && landmark.accepted &&
                                    landmark.found.y+landmark.found.height>.60 &&
                                    (horizontal || result.dy<0))
                                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                        label+": Correction would approach the stair boundary.",stream);
                            }
                        }
                        pbf_move_left_joystick(context,
                            {horizontal ? (result.dx > 0 ? 1.0 : -1.0) : 0.0,
                             horizontal ? 0.0 : (result.dy < 0 ? 1.0 : -1.0)}, duration, 0ms);
                        pbf_wait(context, 500ms);
                    } else {
                        confirmations = 0;
                        if (matcher==match_hearthome_center_entrance || matcher==match_hearthome_contest_fantina){
                            // Do not sweep into a different doorway or around the Hall's
                            // side when a cloud, NPC or fade hides the required anchors.
                            if (++missing>20) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                label+": Scene anchors missing; stationary reacquisition exhausted.",stream);
                            pbf_wait(context,500ms);
                            context.wait_for_all_requests();
                            continue;
                        }
                        if (matcher==match_galactic_hq_jupiter || matcher==match_route208_stair_top){
                            // Keep generic missing-anchor behavior unchanged. Route208's
                            // explicit caller can instead request its recorded recovery.
                            const bool stair_rocks=matcher==match_route208_stair_top;
                            if (stair_rocks && rock_acquisition_timed_out) {
                                const auto now=std::chrono::steady_clock::now();
                                if (missing_rocks_since==std::chrono::steady_clock::time_point::max())
                                    missing_rocks_since=now;
                                // Matcher processing can take seconds per observation;
                                // measure real elapsed time rather than counting waits.
                                if (now-missing_rocks_since>=10s) {
                                    context.wait_for_all_requests();
                                    *rock_acquisition_timed_out=true;
                                    stream.log("["+label+"] Rock acquisition timed out after 10s; handing off to the requested cliff recovery.");
                                    return;
                                }
                            } else if (++missing>(stair_rocks ? 20 : 6)) {
                                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                                    label+(stair_rocks ? ": Rock anchors missing; no blind search near stairs." :
                                        ": Chair/stair anchors missing; no blind search near stairs."),stream);
                            }
                            pbf_wait(context,500ms);
                            context.wait_for_all_requests();
                            continue;
                        }
                        // Search right first, then across the starting point to the
                        // left and back. Each complete sweep returns to its origin.
                        // Try that sweep at the initial height, above it, and below it;
                        // no missing/weak landmark ever confirms the destination.
                        if (++missing > (search_down_first ? 78 : 74)) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            label + ": Landmarks missing during bounded bidirectional search.", stream);
                        if (search_down_first && missing <= 4) {
                            stream.log("[" + label + "] Post-battle acquisition: moving down before lateral search.");
                            pbf_move_left_joystick(context, {0.0, -1.0}, 100ms, 0ms);
                        } else {
                            const size_t search_step = missing - 1 - (search_down_first ? 4 : 0);
                            const size_t level = search_step / 25;
                            const size_t slot = search_step % 25;
                            const bool lateral = slot < 24;
                            const double direction = slot < 6 || slot >= 18 ? 1.0 : -1.0;
                            const auto search_duration = lateral ? 100ms : level == 0 ? 600ms : 1200ms;
                            stream.log("[" + label + "] Acquisition search step=" + std::to_string(missing) +
                                ", direction=" + (lateral ? (direction>0 ? "right" : "left") : (level==0 ? "up" : "down")));
                            // On the street, a northward search from the east
                            // edge can stop at the building/NPC collision row.
                            // Step west around it before changing height, then
                            // return to the same lane for the next bounded sweep.
                            const bool street_height_search=!lateral && matcher==match_jubilife_street_approach;
                            if (street_height_search) pbf_move_left_joystick(context,{-1.0,0.0},150ms,0ms);
                            pbf_move_left_joystick(context,
                                {lateral ? direction : 0.0, lateral ? 0.0 : (level == 0 ? 1.0 : -1.0)}, search_duration, 0ms);
                            if (street_height_search) pbf_move_left_joystick(context,{1.0,0.0},150ms,0ms);
                        }
                        pbf_wait(context, 500ms);
                    }
                    context.wait_for_all_requests();
                }
                OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                    label + ": Could not confirm the reference position.", stream);
            }

            void fight_trainer_using_moves(VideoStream& stream, ProControllerContext& context,
                const std::vector<size_t>& moves, const std::string& label, bool post_battle_dialogue_cleanup) {
                fight_using_move_list(stream,context,BattleType::TRAINER,moves,label,true,post_battle_dialogue_cleanup);
            }

            void fight_trainer_using_moves(VideoStream& stream, ProControllerContext& context,
                size_t move, const std::string& label, bool post_battle_dialogue_cleanup) {
                fight_trainer_using_moves(stream,context,std::vector<size_t>{move},label,post_battle_dialogue_cleanup);
            }

            void fight_trainer_using_move_slot(VideoStream& stream, ProControllerContext& context,
                size_t move_slot, const std::string& label) {
                fight_using_move_slot(stream,context,BattleType::TRAINER,move_slot,label);
            }

            void fight_tristan(VideoStream& stream, ProControllerContext& context) {
                // Chimchar learns Ember into slot 3 at level 6. Confirm the move
                // cursor visually; missing slot 3 fails instead of using Scratch.
                fight_using_move_slot(stream, context, BattleType::TRAINER, 3, "Tristan (Ember)");
            }

            void battle_1_pokemon(VideoStream& stream, ProControllerContext& context) {
                stream.log("[Training] Finding one wild Pokemon using the catch encounter wiggle.", COLOR_BLUE);
                encounter_one_wild_pokemon(stream, context);
                // Use the same battle loop as Tristan, but Scratch is slot 1.
                fight_using_move_slot(stream, context, BattleType::STANDARD, 1, "Training (Scratch)");
                stream.log("[Training] Fight and level-up cleanup complete; verifying Potion healing.");
                use_potion_first_pokemon(stream, context);
                context.wait_for_all_requests();
                stream.log("[Training] One wild battle completed; slot 1 healing verified.", COLOR_GREEN);
            }

            // ---------------------------------------------------------------------------
            // Catch 1 Pokemon
            // ---------------------------------------------------------------------------

            void catch_1_pokemon(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                stream.log("[AutoStory] Starting catch_1_pokemon helper...", COLOR_BLUE);

                encounter_one_wild_pokemon(stream, context);

                // Catch the pokemon.
                stream.log("[AutoStory] Throwing Poke Ball (manual bag open)...", COLOR_BLUE);
                ReceivePokemonDetector received_pokemon;
                // Preserve exit-transition state from the ball outcome through
                // receipt/experience cleanup; do not recreate it between phases.
                EndBattleWatcher battle_ended;
                bool caught = false;
                bool exit_seen = false;
                for (int throw_attempt = 0; throw_attempt < 10; throw_attempt++) {
                    // Open bag from battle menu (X), confirm first slot (ZL).
                    pbf_press_button(context, BUTTON_X, 160ms, 840ms);
                    context.wait_for_all_requests();
                    pbf_press_button(context, BUTTON_ZL, 160ms, 1000ms);
                    context.wait_for_all_requests();

                    // Wait to see if the battle menu comes back (ball failed)
                    // or if the battle ends (caught or fainted).
                    BattleMenuWatcher menu_back(BattleType::STANDARD, COLOR_YELLOW);
                    int throw_ret = run_until<ProControllerContext>(
                        stream, context,
                        [](ProControllerContext& ctx) {
                            pbf_mash_button(ctx, BUTTON_B, 120000ms);
                        },
                        {{menu_back}, {received_pokemon}, {battle_ended}}
                    );

                    if (throw_ret == 1 || (throw_ret == 2 && received_pokemon.received())) {
                        caught = true;
                        exit_seen = throw_ret == 2;
                        stream.log("[AutoStory] Pokemon receipt verified.", COLOR_GREEN);
                        break;
                    }
                    if (throw_ret < 0 || throw_ret == 2) {
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Catch failed: No verified Pokemon receipt before battle ended or timed out.", stream);
                    }
                    stream.log(
                        "[AutoStory] Ball failed, retrying (attempt " + std::to_string(throw_attempt + 2) + ")...",
                        COLOR_ORANGE
                    );
                }

                context.wait_for_all_requests();

                if (!caught) {
                    OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                        "Catch failed: Exhausted ten ball attempts.", stream);
                }
                // Receipt alone is not an overworld return: clear experience,
                // automatic move learning, dex registration and nickname screens
                // until the same completed-black-transition watcher used in battles fires.
                if (!exit_seen) {
                    stream.log("[Catch cleanup] Receipt confirmed; waiting for battle exit transition.");
                    const int cleanup_ret = run_until<ProControllerContext>(stream, context,
                        [](ProControllerContext& ctx) { pbf_mash_button(ctx, BUTTON_B, 120000ms); },
                        {{battle_ended}});
                    if (cleanup_ret != 0) {
                        OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Catch cleanup: Battle exit transition did not finish.", stream);
                    }
                }
                context.wait_for_all_requests();
                stream.log("[Catch cleanup] Battle exit transition confirmed; checking stable overworld.");
                confirm_battle_overworld(stream, context, "Catch cleanup");
                stream.log("[Catch cleanup] Verified receipt, completed battle exit, and four seconds of clear overworld.", COLOR_GREEN);
            }

            // ---------------------------------------------------------------------------
            // catch_1_pokemon_v2 - burst wiggle variant for testing.
            // Uses WIGGLES_PER_BURST consecutive left/right moves before checking for a
            // battle, reducing total wait time compared to checking after every move.
            // Commented out - enable for testing when ready.
            // ---------------------------------------------------------------------------
            // void catch_1_pokemon_v2(
            //     SingleSwitchProgramEnvironment& env,
            //     VideoStream& stream,
            //     ProControllerContext& context
            // ) {
            //     stream.log("[AutoStory] Starting catch_1_pokemon_v2 helper...", COLOR_BLUE);
            //
            //     // Save once before attempting to catch. On failure the game
            //     // resets back to this save, so the save lives outside the loop.
            //     stream.log("[AutoStory] Saving game before catch attempt...", COLOR_BLUE);
            //     {
            //         static constexpr int SAVE_MAX_RETRIES = 3;
            //         bool save_succeeded = false;
            //         for (int save_attempt = 0; save_attempt < SAVE_MAX_RETRIES; save_attempt++) {
            //             try {
            //                 if (save_attempt > 0) {
            //                     stream.log(
            //                         "[AutoStory] Retrying save (attempt " + std::to_string(save_attempt + 1) + ")...",
            //                         COLOR_ORANGE
            //                     );
            //                     pbf_mash_button(context, BUTTON_B, 1000ms);
            //                     context.wait_for_all_requests();
            //                 }
            //                 save_game(stream, context);
            //                 save_succeeded = true;
            //                 break;
            //             }
            //             catch (OperationFailedException& e) {
            //                 stream.log(
            //                     "[AutoStory] Save attempt " + std::to_string(save_attempt + 1) + " failed: " + e.message(),
            //                     COLOR_RED
            //                 );
            //                 context.wait_for(1000ms);
            //             }
            //         }
            //         if (!save_succeeded) {
            //             stream.log("[AutoStory] All save attempts failed. Using blind fallback save.", COLOR_ORANGE);
            //             pbf_mash_button(context, BUTTON_B, 1000ms);
            //             save_game(context);
            //             pbf_mash_button(context, BUTTON_B, 2000ms);
            //         }
            //     }
            //     context.wait_for_all_requests();
            //     pbf_wait(context, 1000ms);
            //
            //     // -------------------------------------------------------
            //     // Mini-checkpoint loop: retry from the save above if we
            //     // run out of balls or our Pokemon faints.
            //     // -------------------------------------------------------
            //     while (true) {
            //
            //     BlackScreenWatcher battle_start;
            //     BattleMenuWatcher battle_menu(BattleType::STANDARD, COLOR_YELLOW);
            //     BattleDialogWatcher battle_dialog(COLOR_YELLOW);
            //
            //     bool encounter_found = false;
            //
            //     // Wiggle left/right in bursts, checking for a battle after each burst.
            //     // Doing multiple moves before checking avoids waiting after every step.
            //     static constexpr int WIGGLES_PER_BURST = 3;
            //     while (!encounter_found) {
            //         for (int i = 0; i < WIGGLES_PER_BURST; i++) {
            //             pbf_move_left_joystick(context, {-1, 0}, 120ms, 80ms);
            //             pbf_move_left_joystick(context, { 1, 0}, 120ms, 80ms);
            //         }
            //         context.wait_for_all_requests();
            //
            //         int ret = wait_until(
            //             stream,
            //             context,
            //             1500ms,
            //             {
            //                 {battle_start},
            //                 {battle_menu},
            //                 {battle_dialog},
            //             }
            //         );
            //
            //         if (ret >= 0) {
            //             encounter_found = true;
            //         }
            //     }
            //
            //     stream.log("[AutoStory] Wild battle detected!", COLOR_GREEN);
            //
            //     // Wait until the actual battle menu appears.
            //     BattleMenuWatcher confirm_menu(BattleType::STANDARD);
            //
            //     int menu_ret = wait_until(
            //         stream,
            //         context,
            //         15s,
            //         {
            //             {confirm_menu},
            //         }
            //     );
            //
            //     if (menu_ret < 0) {
            //         stream.log(
            //             "[AutoStory] Failed to detect battle menu. Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //     }
            //
            //     stream.log("[AutoStory] Battle menu detected.", COLOR_GREEN);
            //
            //     context.wait_for_all_requests();
            //     pbf_wait(context, 500ms);
            //
            //     // --- Throw balls ---
            //     CatchResults result = basic_catcher(
            //         stream,
            //         context,
            //         Language::English,
            //         "poke-ball",
            //         10
            //     );
            //
            //     switch (result.result) {
            //     case CatchResult::POKEMON_CAUGHT:
            //         stream.log(
            //             "[AutoStory] Pokemon successfully caught after " +
            //             std::to_string(result.balls_used) + " ball(s).",
            //             COLOR_GREEN
            //         );
            //         {
            //             OverworldWatcher overworld;
            //             run_until<ProControllerContext>(
            //                 stream, context,
            //                 [](ProControllerContext& ctx) {
            //                     pbf_mash_button(ctx, BUTTON_B, 120000ms);
            //                 },
            //                 {{overworld}}
            //             );
            //         }
            //         return;
            //
            //     case CatchResult::POKEMON_FAINTED:
            //         stream.log("[AutoStory] Wild Pokemon fainted. Exiting battle.", COLOR_ORANGE);
            //         {
            //             OverworldWatcher overworld;
            //             run_until<ProControllerContext>(
            //                 stream, context,
            //                 [](ProControllerContext& ctx) {
            //                     pbf_mash_button(ctx, BUTTON_B, 120000ms);
            //                 },
            //                 {{overworld}}
            //             );
            //         }
            //         return;
            //
            //     case CatchResult::OUT_OF_BALLS:
            //         stream.log(
            //             "[AutoStory] Ran out of Poke Balls after " +
            //             std::to_string(result.balls_used) + " throw(s). Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //
            //     case CatchResult::OWN_FAINTED:
            //         stream.log(
            //             "[AutoStory] Own Pokemon fainted. Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //
            //     case CatchResult::BALL_LIMIT_REACHED:
            //         stream.log(
            //             "[AutoStory] Ball limit reached (" +
            //             std::to_string(result.balls_used) + " throw(s)). Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //
            //     case CatchResult::CANNOT_THROW_BALL:
            //         stream.log(
            //             "[AutoStory] Cannot throw ball (menu timing issue). Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //
            //     default:
            //         stream.log(
            //             "[AutoStory] Catch timed out after " +
            //             std::to_string(result.balls_used) + " ball(s). Resetting to checkpoint...",
            //             COLOR_RED
            //         );
            //         reset_game_from_home(env, env.console, context, true);
            //         continue;
            //     }
            //
            //     } // end while(true)
            // }

            // ---------------------------------------------------------------------------
            // Movement Helpers
            // ---------------------------------------------------------------------------

            bool detect_black_screen_after_movement(
                VideoStream& stream,
                ProControllerContext& context,
                Milliseconds timeout
            ) {
                BlackScreenWatcher black_screen;
                return wait_until(stream, context, timeout, {{black_screen}}) == 0;
            }

            bool try_right_down_exit_search(
                VideoStream& stream,
                ProControllerContext& context,
                size_t iterations
            ) {
                for (size_t i = 0; i < iterations; i++) {
                    pbf_move_left_joystick(context, { 1.0, 0.0 }, 80ms, 0ms);
                    pbf_move_left_joystick(context, { 0.0, -1.0 }, 220ms, 0ms);

                    if (detect_black_screen_after_movement(stream, context, 1000ms)) {
                        return true;
                    }
                }
                return false;
            }

            void leave_pokemon_center_before_barry_1(
                VideoStream& stream,
                ProControllerContext& context
            ) {
                // Original down movement to leave Pokemon Center.
                pbf_move_left_joystick(context, { 0.0, -1.0 }, 814ms, 0ms);

                if (detect_black_screen_after_movement(stream, context, 1200ms)) {
                    pbf_wait(context, 2340ms);
                    return;
                }

                stream.log("[AutoStory] Pokemon Center exit black screen not detected. Searching with right/down movement.", COLOR_ORANGE);

                if (try_right_down_exit_search(stream, context, 20)) {
                    pbf_wait(context, 2340ms);
                    return;
                }

                stream.log("[AutoStory] First exit search failed. Repositioning left and retrying.", COLOR_ORANGE);
                pbf_move_left_joystick(context, { -1.0, 0.0 }, 5000ms, 0ms);

                if (try_right_down_exit_search(stream, context, 30)) {
                    pbf_wait(context, 2340ms);
                    return;
                }

                OperationFailedExceptionWithScreenshot::fire(
                    ErrorReportMode::SEND_ERROR_REPORT,
                    "Failed to find Pokemon Center exit black screen before Barry 1. Triggering checkpoint retry.",
                    stream
                );
            }

            // ---------------------------------------------------------------------------
            // Generic Movement Helper - Walk through grass in specified direction for
            // specified steps, attempting to run from any battles that are encountered.
            // ---------------------------------------------------------------------------
            void walk_through_grass(
                VideoStream& stream, ProControllerContext& context,
                int8_t x, int8_t y, size_t steps, const std::string& direction_name,
                BattleType battle_type, Milliseconds movement_duration,
                OverworldMatchResult (*destination)(const ImageRGB32&)
            ) {
                size_t movements = 0, confirmations = 0;
                while (true) {
                    context.wait_for_all_requests();
                    // Destination checks happen before input and after each step/escape.
                    // With no matcher, retain the original fixed-step behavior.
                    VideoOverlaySet overlays(stream.overlay());
                    OverworldMatchResult location;
                    if (destination) {
                        const auto frame = stream.video().snapshot();
                        if (!frame) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Grass destination: No video frame.", stream);
                        location = destination(*frame.frame);
                        for (const auto& landmark : location.landmarks) {
                            overlays.add(COLOR_CYAN, landmark.search);
                            if (landmark.accepted) overlays.add(COLOR_GREEN, landmark.found);
                            stream.log("[Grass destination] " + direction_name + " " + landmark.name +
                                " score=" + std::to_string(landmark.score) +
                                " margin=" + std::to_string(landmark.margin) +
                                " accepted=" + std::to_string(landmark.accepted));
                        }
                        stream.log("[Grass destination] located=" + std::to_string(location.located) +
                            " at_target=" + std::to_string(location.at_target) +
                            " screen-relative offset=(" + std::to_string(location.dx) +
                            "," + std::to_string(location.dy) + ")");
                        if (location.at_target) {
                            if (++confirmations >= 3) {
                                stream.log("[Grass] Destination confirmed; stopped " + direction_name +
                                    " after " + std::to_string(movements) + "/" + std::to_string(steps) + " movements.", COLOR_GREEN);
                                return;
                            }
                            pbf_wait(context, 250ms);
                            continue;
                        }
                        confirmations = 0;
                    }
                    if (movements >= steps) {
                        if (destination) OperationFailedExceptionWithScreenshot::fire(ErrorReportMode::SEND_ERROR_REPORT,
                            "Grass destination not confirmed within movement limit: " + direction_name, stream);
                        stream.log("[Grass] Completed " + direction_name + " (" + std::to_string(steps) + " steps).", COLOR_GREEN);
                        return;
                    }
                    int8_t move_x=x, move_y=y;
                    Milliseconds duration=movement_duration;
                    if (destination && location.located && !location.coarse &&
                        std::abs(location.dx)<=0.10 && std::abs(location.dy)<=0.12) {
                        // Small corrections prevent stepping past a narrow stop target.
                        const bool horizontal=std::abs(location.dx)>=std::abs(location.dy)*9.0/16;
                        const double error=horizontal ? location.dx : location.dy*9.0/16;
                        duration=std::min(duration, std::abs(error)>0.03125 ? 80ms :
                            std::abs(error)>0.015625 ? 40ms : 16ms);
                        move_x=horizontal ? (location.dx>0 ? 1 : -1) : 0;
                        move_y=horizontal ? 0 : (location.dy<0 ? 1 : -1);
                        stream.log("[Grass destination] Correcting " + std::string(horizontal ? "horizontal" : "vertical") +
                            " axis for " + std::to_string(duration.count()) + "ms.");
                    }
                    // Each movement gets fresh watcher state. A completed white
                    // transition from a prior encounter must not trigger again.
                    BlackScreenWatcher battle_start;
                    WhiteScreenOverWatcher white_screen(COLOR_YELLOW, {0.1, 0.1, 0.8, 0.8});
                    BattleMenuWatcher battle_menu(battle_type, COLOR_YELLOW);
                    BattleDialogWatcher battle_dialog(COLOR_YELLOW);
                    pbf_move_left_joystick(context, {(double)move_x, (double)move_y}, duration, 0ms);
                    movements++;
                    context.wait_for_all_requests();
                    int ret = wait_until(stream, context, 2500ms,
                        {{white_screen}, {battle_start}, {battle_dialog}, {battle_menu}});
                    if (ret >= 0) {
                        confirmations = 0;
                        stream.log("[Grass] " + direction_name + " step " + std::to_string(movements) +
                            "/" + std::to_string(steps) + ": encounter trigger=" + std::to_string(ret) +
                            "; using shared confirmed-Run escape.");
                        escape_wild_battle(stream, context, battle_type);
                        context.wait_for_all_requests();
                        stream.log("[Grass] Escape confirmed; resuming " + direction_name + ".");
                    }
                }
            }

        }
    }
}
