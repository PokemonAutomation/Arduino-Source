/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh_MaxLair_Run_Entrance.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_entrance(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    bool followed_path,
    GlobalStateTracker& state_tracker
){
    GlobalState& state = state_tracker[console_index];

    if (!state.adventure_started){
        stream.log("Failed to start raid.", COLOR_RED);
        runtime.session_stats.add_error();
    }else if (state.wins == 0){
        stream.log("Lost on first raid.", COLOR_PURPLE);
        runtime.session_stats.add_run(0);
        if (console_index == runtime.host_index){
            runtime.path_stats.clear();
        }
    }

    context.wait_for(1000ms);

    // Get the boss slug
    std::string boss_slug;
    if (runtime.host_index < runtime.console_settings.active_consoles()){
        boss_slug = state_tracker.infer_actual_state(runtime.host_index).boss;
    };
    
    bool save_path = false;
    
    if (!followed_path){
        // Check if the user checked the box to save the path when running the BossFinder program
        
        if (!boss_slug.empty()){
            save_path = runtime.actions.is_in_save_list(boss_slug);
            stream.log("Boss: " + boss_slug + ", should save: " + (save_path ? "Yes" : "No"), COLOR_BLUE);
        }
    }else{
        save_path = followed_path;
    }


    while (true){
        WhiteDialogBoxWatcher dialog;
        SelectionArrowFinder arrow(stream.overlay(), {0.462377, 0.332039, 0.388222, 0.640777});
        YCommIconWatcher overworld;

        context.wait_for_all_requests();
        context.wait_for(100ms);

        int ret = wait_until(
            stream, context,
            std::chrono::seconds(10),
            {
                dialog,
                arrow,
                overworld,
            }
        );
        switch (ret){
        case 0:
            stream.log("Detected dialog menu.");
            pbf_press_button(context, BUTTON_B, 80ms, 160ms);
            continue;
        case 1:{
            stream.log("Detected arrow.");
            if (!save_path){
                pbf_press_button(context, BUTTON_B, 80ms, 160ms);
                continue;
            }
            const std::vector<ImageFloatBox>& arrows = arrow.last_detection();
            if (arrows.empty()){
                continue;
            }
            if (arrows[0].y > 0.56){
                pbf_press_button(context, BUTTON_A, 80ms, 160ms);
                continue;
            }

            // List of bosses is full, stop the program
            stream.log("Cannot save path – saved list is full. Stopping program.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Paths list is full. Program stopped.",
                stream
            );
        }
        case 2:
            stream.log("Detected overworld.");
            return;
        default:
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                "No recognized state after 10 seconds.",
                stream
            );
        }
    }
}



}
}
}
}
