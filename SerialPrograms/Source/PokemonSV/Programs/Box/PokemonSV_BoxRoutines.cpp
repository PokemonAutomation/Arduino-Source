/*  Sandwich Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_BoxRoutines.h"


#include <cmath>
#include <algorithm>
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void hold_one_column(BotBaseContext& context){
     // Minus to draw selection box
    pbf_press_button(context, BUTTON_MINUS, 20, 40);
    // Select rest of the pary
    for(int i = 0; i < 4; i++){
        pbf_press_dpad(context, DPAD_DOWN, 10, 40);
    }
    // Hold rest of the pary
    pbf_press_button(context, BUTTON_A, 20, 40);
}

bool release_one_pokemon(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context, bool ensure_empty){
    context.wait_for_all_requests();
    BoxSelectWatcher selected(COLOR_RED);
    // If no pokemon in the slot:
    if (!selected.exists(console.video().snapshot())){
        console.log("Slot is empty.");
        return false;
    }
    {
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 230);
            },
            {selected}
        );
        if (ret < 0){
            dump_image(
                console, env.program_info(),
                "SelectionArrowNotFound",
                console.video().snapshot()
            );
            throw OperationFailedException(console.logger(), "Unable to detect selection arrow.");
        }
    }
    context.wait_for(std::chrono::milliseconds(100));

    pbf_press_dpad(context, DPAD_UP, 20, 30);
    pbf_press_dpad(context, DPAD_UP, 20, 30);
    context.wait_for_all_requests();

    console.log("Releasing...");
    console.overlay().add_log("Release pokemon", COLOR_PURPLE);
    {
        PromptDialogWatcher confirm(COLOR_CYAN);
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 230);
            },
            {confirm}
        );
        if (ret < 0){
            dump_image(
                console, env.program_info(),
                "ConfirmationNotFound",
                console.video().snapshot()
            );
            throw OperationFailedException(console.logger(), "Release confirmation not found.");
        }
        context.wait_for(std::chrono::milliseconds(500));
    }

    pbf_press_dpad(context, DPAD_UP, 20, 30);
    context.wait_for_all_requests();

    console.log("Waiting for release to finish...");
    if (ensure_empty) {
        // Detect when there is no longer pokemon in the current slot
        SomethingInBoxSlotWatcher done_releasing(COLOR_RED, false);
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);
            },
            {done_releasing}
        );
        if (ret < 0){
            dump_image(
                console, env.program_info(),
                "ReleaseFailed",
                console.video().snapshot()
            );
            throw OperationFailedException(console.logger(), "Unable to release pokemon.");
        }
        context.wait_for(std::chrono::milliseconds(100));
    } else {
        pbf_mash_button(context, BUTTON_A, 60);
        AdvanceDialogWatcher advance_dialog(COLOR_RED);
        int ret = wait_until(
            console, context,
            std::chrono::seconds(10),
            {advance_dialog}
        );
        if (ret < 0){
            dump_image(
                console, env.program_info(),
                "AfterReleaseDialogNotFound",
                console.video().snapshot()
            );
            throw OperationFailedException(console.logger(), "Unable to find after-release advance dialog.");
        }
        pbf_wait(context, 100);
        pbf_mash_button(context, BUTTON_A, 60);
        pbf_wait(context, 200);
        context.wait_for_all_requests();
        // console.overlay().add_log("Released", COLOR_WHITE);
        // context.wait_for(std::chrono::seconds(30));
    }
    
    return true;
}

uint8_t check_empty_slots_in_party(ConsoleHandle& console, BotBaseContext& context){
    context.wait_for_all_requests();
    BoxEmptyPartyWatcher watcher;
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
        {watcher}
    );
    if (ret < 0){
        throw OperationFailedException(console.logger(), "check_empty_slots_in_party(): Cannot read party emptyness in box system.");
    }

    return watcher.num_empty_slots_found();
}

void load_one_column_to_party(ConsoleHandle& console, BotBaseContext& context, uint8_t column_index, bool has_clone_ride_pokemon){
    BoxDetector box_detector;
    // Move cursor to the current column
    box_detector.move_cursor(console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);

    hold_one_column(context);
    // Move the held column to party
    box_detector.move_cursor(console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);

    // Drop the column to party
    pbf_press_button(context, BUTTON_A, 20, 80);

    context.wait_for_all_requests();
}

void unload_one_column_from_party(ConsoleHandle& console, BotBaseContext& context, uint8_t column_index, bool has_clone_ride_pokemon){
    BoxDetector box_detector;
    // Move cursor to party column
    box_detector.move_cursor(console, context, BoxCursorLocation::PARTY, has_clone_ride_pokemon ? 2 : 1, 0);

    hold_one_column(context);

    // Move the held column to target
    box_detector.move_cursor(console, context, BoxCursorLocation::SLOTS, has_clone_ride_pokemon ? 1 : 0, column_index);

    // Drop the column
    pbf_press_button(context, BUTTON_A, 20, 80);

    context.wait_for_all_requests();
}


}
}
}
