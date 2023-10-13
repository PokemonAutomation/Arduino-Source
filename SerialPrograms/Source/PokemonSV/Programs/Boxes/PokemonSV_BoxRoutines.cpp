/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <algorithm>
#include <sstream>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV_BoxRoutines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool change_view_to_stats_or_judge(
    ConsoleHandle& console, BotBaseContext& context,
    bool throw_exception
){
    ImageFloatBox name_bar(0.66, 0.08, 0.52, 0.04);
    OverlayBoxScope name_bar_overlay(console.overlay(), name_bar);
    for (size_t attempts = 0;; attempts++){
        if (throw_exception){
            if (attempts == 10){
                throw OperationFailedException(
                    ErrorReport::SEND_ERROR_REPORT, console,
                    "Unable to change Pokemon view after 10 tries.",
                    true
                );
            }
        }else{
            if (attempts == 3){
                return false;
            }
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        ImageStats stats = image_stats(extract_box_reference(screen, name_bar));
//        cout << stats.stddev << endl;
        if (stats.stddev.sum() > 50){
            break;
        }

        console.log("Unable to detect stats menu. Attempting to correct.", COLOR_RED);

//        //  Alternate one and two + presses. If IV checker is enabled, we should
//        //  land on the IV checker. Otherwise, it will land us back to nothing.
//        //  Then the next press will be a single which will put us on the stats
//        //  with no IV checker.
        pbf_press_button(context, BUTTON_PLUS, 20, 105);
//        if (attempts % 2 == 0){
//            pbf_press_button(context, BUTTON_PLUS, 20, 230);
//        }
    }
    return true;
}


void change_view_to_judge(
    ConsoleHandle& console, BotBaseContext& context,
    Language language
){
    if (language == Language::None){
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "change_view_to_judge() called with no language."
        );
    }

    ImageFloatBox name_bar(0.66, 0.08, 0.52, 0.04);
    IvJudgeReaderScope iv_checker(console, language);
    OverlayBoxScope name_bar_overlay(console.overlay(), name_bar);
    for (size_t attempts = 0;; attempts++){
        if (attempts == 5){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Unable to change Pokemon view to judge after 10 tries. Have you unlocked it?",
                true
            );
        }

        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        ImageStats stats = image_stats(extract_box_reference(screen, name_bar));
//        cout << stats.stddev << endl;

        //  Check if we're even on a stats screen.
        if (stats.stddev.sum() < 50){
            console.log("Unable to detect stats menu. Attempting to correct.", COLOR_RED);
            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            continue;
        }

        //  See if we're on the judge screen.
        IvJudgeReader::Results ivs = iv_checker.read(console, screen);

        size_t detected = 0;
        if (ivs.hp      != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.attack  != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.defense != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.spatk   != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.spdef   != IvJudgeValue::UnableToDetect) detected++;
        if (ivs.speed   != IvJudgeValue::UnableToDetect) detected++;

        //  If less than 4 of the IVs are read, assume we're not on the judge screen.
        if (detected < 4){
            pbf_press_button(context, BUTTON_PLUS, 20, 230);
        }

        break;
    }
}



// Moving to left/right box is blind sequence. To prevent game dropping button inputs,
// press the button longer.
void move_to_left_box(BotBaseContext& context){
    pbf_press_button(context, BUTTON_L, 60, 100);
}
void move_to_right_box(BotBaseContext& context){
    pbf_press_button(context, BUTTON_R, 60, 100);
}

namespace {

// In box system, when using button Minus or button Y, it will enter a mode for box selection and holding pokemon.
// The function detects the existence of button symbol in the bottom row of the screen to make
// sure the mode is entered or left.
// If `enter_mode` is true, the program will assume it is not in the mode, and enter the mode by calling
// `button_operation()` repeatedly until the mode is observed.
// If `enter_mode` is false, the program will assume it is in the mode, and leave the mode by calling
// `button_operation()` repeatedly until it confirms to have left the mode.
void change_held_mode(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    std::function<void()> button_operation, bool enter_mode,
    const char* time_out_error_name, const char* time_out_error_message)
{
    // First attempt to change mode
    button_operation();

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, console, time_out_error_name, time_out_error_message
            );
        }
        BoxSelectionBoxModeWatcher watcher;
        context.wait_for_all_requests();
        if (wait_until(console, context, std::chrono::seconds(10), {watcher}) == 0){
            if (watcher.in_box_selection_mode() == enter_mode){
                break;
            }
            // else: not in the desired mode
            button_operation();
            continue;
        }
        // Cannot detect box selection mode after 10 secs
        dump_image_and_throw_recoverable_exception(
            info, console, "NoStateBoxSelectionMode", "Cannot determine box selection mode for 10 seconds."
        );
    }
}

} // anonymous namesapce

// Use box selection mode to hold one column of pokemon.
// Use visual feedback to ensure button Minus is pressed to turn on box selection mode/
// But no feedback on the button A press to make sure the selection is complete.
void hold_one_column(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Holding one column.");
    // Press Minus to draw selection box
    const bool to_enter_selection = true;
    change_held_mode(
        info, console, context,
        [&context](){ pbf_press_button(context, BUTTON_MINUS, 50, 40); },
        to_enter_selection,
        "TimeoutHoldingColumn", "Failed to enter box selection mode after 1 minute of Button Minus pressing."
    );

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
    context.wait_for_all_requests();
    console.log("Drop held pokemon.");
    const bool to_enter_selection = false;
    change_held_mode(
        info, console, context,
        [&context](){ pbf_press_button(context, BUTTON_A, 60, 60); },
        to_enter_selection,
        "TimeoutDroppingPokemon", "Failed to drop pokemon after 1 minute of Button A pressing."
    );
}

// Assuming already holding one or more pokemon, press B to cancel the holding.
void cancel_held_pokemon(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Cancel pokemon holding.");
    const bool to_enter_selection = false;
    change_held_mode(
        info, console, context,
        [&context](){ pbf_press_button(context, BUTTON_B, 60, 60); },
        to_enter_selection,
        "TimeoutCancellingHolding", "Failed to cancel holding pokemon after 1 minute of Button B pressing."
    );
}

// Assuming the current slot is not empy, press button Y to hold the current pokemon
void press_y_to_hold_pokemon(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    console.log("Press button Y to hold pokemon for swapping.");
    const bool to_enter_selection = true;
    change_held_mode(
        info, console, context,
        [&context](){ pbf_press_button(context, BUTTON_Y, 60, 60); },
        to_enter_selection,
        "TimeoutHoldingPokemonByButtonY", "Failed to hold a pokemon by button Y after 1 minute of trying."
    );
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
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
){
    context.wait_for_all_requests();
    console.log("Load column " + std::to_string(column_index) + " to party.");
    console.overlay().add_log("Load column " + std::to_string(column_index+1), COLOR_WHITE);

    size_t fail_count = 0;
    while (true){
        // Move cursor to the target column
        move_box_cursor(env.program_info(), console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);
        hold_one_column(env.program_info(), console, context);
        try{
            // Move the held column to party
            move_box_cursor(env.program_info(), console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);
        }catch (OperationFailedException& e){
            e.send_notification(env, notification);

            if (++fail_count == 10){
                dump_image_and_throw_recoverable_exception(
                    env.program_info(), console, "ConsecutiveColumnMoveFailure",
                    "load_one_column_to_party(): consecutive failure of moving column around."
                );
            }
            console.log("Failed to move column around. Cancelling box selection and try again.");
            // Cannot move column to party. It could be that the game dropped the button A press that is expected to finish
            // the column selection.
            // We can press B to back out and try again.
            cancel_held_pokemon(env.program_info(), console, context);
            continue;
        }
        break;
    }

    // Drop the column to party
    drop_held_pokemon(env.program_info(), console, context);

    context.wait_for_all_requests();
    console.log("Loaded column " + std::to_string(column_index) + " to party.");
}

void unload_one_column_from_party(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
){
    context.wait_for_all_requests();
    console.log("Unload party to column " + std::to_string(column_index) + ".");
    console.overlay().add_log("Unload to column " + std::to_string(column_index+1), COLOR_WHITE);

    size_t fail_count = 0;
    while (true){
        // Move cursor to party column
        move_box_cursor(env.program_info(), console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);
        hold_one_column(env.program_info(), console, context);

        try{
            // Move the held column to target
            move_box_cursor(env.program_info(), console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);
        }catch (OperationFailedException& e){
            e.send_notification(env, notification);

            if (++fail_count == 10){
                dump_image_and_throw_recoverable_exception(
                    env.program_info(), console, "ConsecutiveColumnMoveFailure",
                    "unload_one_column_from_party(): consecutive failure of moving column around."
                );
            }
            console.log("Failed to move column around. Cancelling box selection and try again.");
            // Cannot move column to party. It could be that the game dropped the button A press that is expected to finish
            // the column selection.
            // We can press B to back out and try again.
            cancel_held_pokemon(env.program_info(), console, context);
            continue;
        }
        break;
    }
    // Drop the column
    drop_held_pokemon(env.program_info(), console, context);

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

    {
        const bool stop_on_exists = true;
        SomethingInBoxSlotWatcher exists(COLOR_BLUE, stop_on_exists);
        if (wait_until(console, context, std::chrono::seconds(3), {exists}) < 0){
            dump_image_and_throw_recoverable_exception(info, console, "EmptySourceSwap", "Swapping an empty slot.");
        }
    }

    press_y_to_hold_pokemon(info, console, context);

    detector.move_cursor(info, console, context, target_side, target_row, target_col);

    drop_held_pokemon(info, console, context);

    console.log("Swapped slots.");
}

}
}
}
