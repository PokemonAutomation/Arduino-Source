/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Run_Entrance.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;


// Read the three currently saved paths (if any) from the entrance screen.
// Returns a vector of three slugs (empty strings for empty slots).


void run_entrance(
    AdventureRuntime& runtime,
    ProgramEnvironment& env, size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    const std::string& boss_slug,
    bool followed_path,
    const EndBattleDecider& decider,
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
    
    // Get the boss slug
    std::string boss_slug;
    if (runtime.host_index < state_tracker.size()) {
        boss_slug = state_tracker.infer_actual_state(runtime.host_index).boss;
    }
    
    // Determine whether we should save this boss
    bool should_save = false;
    if (!boss_slug.empty()) {
        should_save = runtime.actions.save_path(boss_slug);
        stream.log("Boss: " + boss_slug + ", should save: " + (should_save ? "Yes" : "No"), COLOR_BLUE);
    }
    
    // Overlay box to detect when a dialogue box is present (grey area at bottom)

    OverlayBoxScope dialog_box(stream.overlay(), {0.782, 0.850, 0.030, 0.050});
    
    // First step, check if there is a path to be saved
    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    context.wait_for_all_requests();
    
    // Then, if there is something to be saved, check if the next dialogue box allows us to directly save or not
    
    if (followed_path) {
        stream.log("Handling followed-path dialogue.", COLOR_BLUE);
        // Keep the path or not
        if (followed_path) {
            pbf_press_button(context, BUTTON_A, 160ms, 0ms);
        } else {
            pbf_press_button(context, BUTTON_B, 160ms, 0ms);
        }
        context.wait_for_all_requests();
    
    // Press A to finish dialogue
    
        while (true) {
            context.wait_for(400ms);
            VideoSnapshot screen = stream.video().snapshot();
            if (!screen) continue;
            ImageStats stats = image_stats(extract_box_reference(*screen, dialog_box));
            
            if (!is_grey(stats, 400, 1000)) {
                break; // Dialogue box gone
            }
            pbf_press_button(context, BUTTON_A, 160ms, 0ms);
            context.wait_for_all_requests();
            
        }
}



}
}
}
}
