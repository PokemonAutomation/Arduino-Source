/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <algorithm>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV_BoxRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



void change_stats_view_to_judge(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context
){
    ImageFloatBox name_bar(0.66, 0.08, 0.52, 0.04);
    OverlayBoxScope name_bar_overlay(console.overlay(), name_bar);
#if 1
    for (size_t attempts = 0;; attempts++){
        if (attempts == 10){
            dump_image_and_throw_recoverable_exception(
                info, console, "ChangePokemonView",
                "check_baby_info: Unable to change Pokemon view after 10 tries."
            );
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        ImageStats stats = image_stats(extract_box_reference(screen, name_bar));
        if (stats.stddev.sum() > 100){
            break;
        }

        console.log("Unable to detect stats menu. Attempting to correct.", COLOR_RED);

        //  Alternate one and two + presses. If IV checker is enabled, we should
        //  land on the IV checker. Otherwise, it will land us back to nothing.
        //  Then the next press will be a single which will put us on the stats
        //  with no IV checker.
        pbf_press_button(context, BUTTON_PLUS, 20, 230);
        if (attempts % 2 == 0){
            pbf_press_button(context, BUTTON_PLUS, 20, 230);
        }
    }
#endif
}

// Use box selection mode to hold one column of pokemon
void hold_one_column(BotBaseContext& context){
    // Minus to draw selection box
    pbf_press_button(context, BUTTON_MINUS, 50, 40);
    // Select rest of the pary
    // Press down multiple times to make sure we select full party in case the game drops some presses
    for(int i = 0; i < 20; i++){
        pbf_press_dpad(context, DPAD_DOWN, 5, 3);
    }
    // Hold rest of the party
    pbf_wait(context, 60);
    pbf_press_button(context, BUTTON_A, 50, 40);
}

// Moving to left/right box is blind sequence. To prevent game dropping button inputs,
// press the button longer.
void move_to_left_box(BotBaseContext& context){
    pbf_press_button(context, BUTTON_L, 60, 100);
}
void move_to_right_box(BotBaseContext& context){
    pbf_press_button(context, BUTTON_R, 60, 100);
}

// Use box selection mode to hold one column of pokemon.
// Use visual feedback to ensure button Minus is pressed to turn on box selection mode/
// But no feedback on the button A press to make sure the selection is complete.
void hold_one_column(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    // Minus to draw selection box
    pbf_press_button(context, BUTTON_MINUS, 50, 40);
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, console, "StartHoldingColumn", "Failed to enter box selection mode after 1 minute of Button Minus pressing."
            );
        }
        BoxSelectionBoxModeWatcher watcher;
        context.wait_for_all_requests();
        if (wait_until(console, context, std::chrono::seconds(10), {watcher}) == 0){
            if (watcher.in_box_selection_mode()){
                break;
            }
            // else: not in selection mode, game drop the button minus press, try again
            pbf_press_button(context, BUTTON_MINUS, 50, 40);
            continue;
        }
        // Cannot detect box selection mode after 10 secs
        dump_image_and_throw_recoverable_exception(
            info, console, "CannotDetermineBoxSelectionMode", "Cannot determine box selection mode while holding a column after 10 seconds."
        );
    }

    // Select rest of the pary
    // Press down multiple times to make sure we select full party in case the game drops some presses
    for(int i = 0; i < 15; i++){
        pbf_press_dpad(context, DPAD_DOWN, 5, 3);
    }
    // Hold rest of the party
    pbf_wait(context, 60);
    // We cannot detect whether this Button A will be dropped or not.
    // So we have to go blind here.
    pbf_press_button(context, BUTTON_A, 50, 40);
    context.wait_for_all_requests();
}

// Assuming already holding one or more pokemon, press A to drop them.
void drop_held_pokemon(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    pbf_press_button(context, BUTTON_A, 60, 60);
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, console, "DropColumn", "Failed to drop a column after 1 minute of Button A pressing."
            );
        }
        BoxSelectionBoxModeWatcher watcher;
        context.wait_for_all_requests();
        if (wait_until(console, context, std::chrono::seconds(10), {watcher}) == 0){
            if (!watcher.in_box_selection_mode()){
                break;
            }
            // else: still in selection mode, game drop the button A press, try again
            pbf_press_button(context, BUTTON_A, 60, 60);
            continue;
        }
        // Cannot detect box selection mode after 10 secs
        dump_image_and_throw_recoverable_exception(
            info, console, "CannotDetermineBoxSelectionMode", "Cannot determine box selection mode while dropping a column after 10 seconds."
        );
    }
}

// Assuming already holding one or more pokemon, press B to cancel the holding.
void cancel_held_pokemon(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    console.log("Cancel pokemon holding.");

    pbf_press_button(context, BUTTON_B, 60, 60);
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, console, "CancelBoxSelection", "Failed to cancel box selection after 1 minute of Button B pressing."
            );
        }
        BoxSelectionBoxModeWatcher watcher;
        context.wait_for_all_requests();
        if (wait_until(console, context, std::chrono::seconds(10), {watcher}) == 0){
            if (!watcher.in_box_selection_mode()){
                break;
            }
            // else: still in selection mode, game drop the button B press, try again
            pbf_press_button(context, BUTTON_B, 60, 60);
            continue;
        }
        // Cannot detect box selection mode after 10 secs
        dump_image_and_throw_recoverable_exception(
            info, console, "CannotDetermineBoxSelectionMode", "Cannot determine box selection mode while cancelling box selection after 10 seconds."
        );
    }
}

void release_one_pokemon(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t& errors
){
    errors = 0;

    bool release_attempted = false;
    bool release_completed = false;
    int expected = 0;
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, console, "ReleaseFailed",
                "Failed to release " + Pokemon::STRING_POKEMON + " after 1 minute."
            );
        }

        SomethingInBoxSlotDetector exists(COLOR_BLUE);
        BoxWatcher box_detector(COLOR_RED);
        BoxSelectWatcher selected(COLOR_YELLOW, std::chrono::milliseconds(100));
        PromptDialogWatcher confirm(COLOR_CYAN, std::chrono::milliseconds(100));
        AdvanceDialogWatcher advance_dialog(COLOR_GREEN, std::chrono::milliseconds(250));

        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(10),
            {
                box_detector,
                selected,
                confirm,
                advance_dialog,
            }
        );
        context.wait_for(std::chrono::milliseconds(50));
        if (ret == 3){
            //  Make sure we're not mistakening this for the other dialogs.
            auto screenshot = console.video().snapshot();
            if (selected.detect(screenshot)){
                ret = 1;
            }else if (confirm.detect(screenshot)){
                ret = 2;
            }
        }

        switch (ret){
        case 0:{
            if (ret == expected){
                console.log("Detected box neutral.");
            }else{
                console.log("Detected box neutral. (unexpected)", COLOR_RED);
                errors++;
            }
            auto screenshot = console.video().snapshot();
            if (exists.detect(screenshot)){
                if (release_attempted && release_completed){
                    return;
                }
                release_attempted = false;
                release_completed = false;
                pbf_press_button(context, BUTTON_A, 20, 20);
                expected = 1;
                continue;
            }else{
                console.log("Slot is empty.");
                return;
            }
        }
        case 1:
            if (ret == expected){
                console.log("Detected selection. Releasing...");
            }else{
                console.log("Detected selection. Releasing... (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_button(context, BUTTON_A, 20, 20);
            expected = 2;
            continue;
        case 2:
            if (ret == expected){
                console.log("Detected release confirmation.");
            }else{
                console.log("Detected release confirmation. (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_dpad(context, DPAD_UP, 10, 10);
            pbf_press_button(context, BUTTON_A, 20, 20);
            release_attempted = true;
            expected = 3;
            continue;
        case 3:
            if (ret == expected){
                console.log("Detected advance dialog.");
            }else{
                console.log("Detected advance dialog. (unexpected)", COLOR_RED);
                errors++;
            }
            pbf_press_button(context, BUTTON_A, 20, 20);
            release_completed = true;
            expected = 0;
            continue;
        default:
            dump_image_and_throw_recoverable_exception(
                info, console, "NoStateReleasingPokemon", "No recognized state while releasing a pokemon after 10 seconds."
            );
        }
    }
}

uint8_t check_empty_slots_in_party(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    BoxEmptyPartyWatcher watcher;
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
        {watcher}
    );
    if (ret < 0){
        dump_image_and_throw_recoverable_exception(
            info, console, "ReadSlotEmptinessFailed", "check_empty_slots_in_party(): Cannot read party emptiness in box system."
        );
    }

    return watcher.num_empty_slots_found();
}

void load_one_column_to_party(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t column_index, bool has_clone_ride_pokemon)
{
    context.wait_for_all_requests();
    console.log("Load column " + std::to_string(column_index) + " to party.");
    console.overlay().add_log("Load column " + std::to_string(column_index+1), COLOR_WHITE);

    size_t fail_count = 0;
    while(true){
        // Move cursor to the target column
        move_box_cursor(info, console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);
        hold_one_column(info, console, context);
        try{
            // Move the held column to party
            move_box_cursor(info, console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);
        } catch(OperationFailedException& e){
            if (++fail_count == 10){
                dump_image_and_throw_recoverable_exception(
                    info, console, "ConsecutiveColumnMoveFailure",
                    "load_one_column_to_party(): consecutive failure of moving column around."
                );
            }
            console.log("Failed to move column around. Cancelling box selection and try again.");
            // Cannot move column to party. It could be that the game dropped the button A press that is expected to finish
            // the column selection.
            // We can press B to back out and try again.
            cancel_held_pokemon(info, console, context);
            continue;
        }
        break;
    }

    // Drop the column to party
    drop_held_pokemon(info, console, context);

    context.wait_for_all_requests();
    console.log("Loaded column " + std::to_string(column_index) + " to party.");
}

void unload_one_column_from_party(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t column_index, bool has_clone_ride_pokemon
){
    context.wait_for_all_requests();
    console.log("Unload party to column " + std::to_string(column_index) + ".");
    console.overlay().add_log("Unload to column " + std::to_string(column_index+1), COLOR_WHITE);

    size_t fail_count = 0;
    while(true){
        // Move cursor to party column
        move_box_cursor(info, console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);
        hold_one_column(info, console, context);

        try{
            // Move the held column to target
            move_box_cursor(info, console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);
        } catch(OperationFailedException& e){
            if (++fail_count == 10){
                dump_image_and_throw_recoverable_exception(
                    info, console, "ConsecutiveColumnMoveFailure",
                    "unload_one_column_from_party(): consecutive failure of moving column around."
                );
            }
            console.log("Failed to move column around. Cancelling box selection and try again.");
            // Cannot move column to party. It could be that the game dropped the button A press that is expected to finish
            // the column selection.
            // We can press B to back out and try again.
            cancel_held_pokemon(info, console, context);
            continue;
        }
        break;
    }
    // Drop the column
    drop_held_pokemon(info, console, context);

    context.wait_for_all_requests();
    console.log("Unloaded party to column " + std::to_string(column_index) + ".");
}

void move_box_cursor(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    const BoxCursorLocation& side, uint8_t row, uint8_t col)
{
    context.wait_for_all_requests();
    console.log("Move cursor to " + BOX_LOCATION_STRING(side, row, col) + ".");
    BoxDetector detector;
    VideoOverlaySet overlay_set(console.overlay());
    detector.make_overlays(overlay_set);
    detector.move_cursor(info, console, context, side, row, col);
    console.log("Cursor moved.");
}

void swap_two_box_slots(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    const BoxCursorLocation& source_side, uint8_t source_row, uint8_t source_col,
    const BoxCursorLocation& target_side, uint8_t target_row, uint8_t target_col
)
{
    context.wait_for_all_requests();
    console.log("Swap two slots " + BOX_LOCATION_STRING(source_side, source_row, source_col)
        + " and " + BOX_LOCATION_STRING(target_side, target_row, target_col) + ".");
    console.overlay().add_log("Swap slots", COLOR_WHITE);

    BoxDetector detector;
    VideoOverlaySet overlay_set(console.overlay());
    detector.make_overlays(overlay_set);

    detector.move_cursor(info, console, context, source_side, source_row, source_col);

    pbf_press_button(context, BUTTON_Y, 60, 60);

    detector.move_cursor(info, console, context, target_side, target_row, target_col);

    pbf_press_button(context, BUTTON_A, 60, 60);

    context.wait_for_all_requests();
    console.log("Swapped slots.");
}

}
}
}
