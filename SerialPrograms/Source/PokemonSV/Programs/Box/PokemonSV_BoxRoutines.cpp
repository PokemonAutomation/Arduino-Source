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

}
}
}
