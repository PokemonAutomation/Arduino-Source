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

namespace {
    // Boxes for the corresponding pokemon names in case the list is full and the player battled a boss that isn't in the list
    const ImageFloatBox NAME_BOXES[3] = {
        {0.30, 0.40, 0.40, 0.05},
        {0.30, 0.50, 0.40, 0.05},
        {0.30, 0.60, 0.40, 0.05}
    };

    // Read the three saved paths names if the user has already saved 3 paths
    std::vector<std::string> read_saved_paths(
                                              VideoStream& stream,
                                              Language language,
                                              const ImageViewRGB32& screen
                                              ) {
        
        std::vector<std::string> slugs;
        
        for (int i = 0; i < 3; ++i) {
            auto cropped = extract_box_reference(screen, NAME_BOXES[i]);
            OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
                                                                                         stream.logger(), language, cropped, OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
                                                                                         0.01, 0.50, 2.0
                                                                                         );
            if (result.results.empty()) {
                slugs.emplace_back();
            } else {
                slugs.push_back(result.results.begin()->second.token);
            }
        }
        return slugs;
    }

    // Read the three currently saved paths (if any) from the entrance screen and return the index of the first slot that is NOT protected, returns -1 otherwise
    int find_unprotected_slot(
                              const std::vector<std::string>& current_slugs,
                              const EndBattleDecider& actions,
                              Logger& logger
                              ) {
        for (int i = 0; i < 3; ++i) {
            if (current_slugs[i].empty()) {
                // Empty slot, override and add error
                logger.log("Failed to read slot " + std::to_string(i) + ", replacing it.", COLOR_RED);
                return i;
            }
            if (!actions.is_protected_path(current_slugs[i])) {
                logger.log("Slot " + std::to_string(i) + " contains unprotected boss, will replace it.", COLOR_BLUE);
                return i;
            }
        }
        logger.log("All slots already saved and protected", COLOR_RED);
        return -1;
    }
}

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
    
    // Get the boss slug
    std::string boss_slug;
    if (runtime.host_index < runtime.console_settings.active_consoles()) {
        boss_slug = state_tracker.infer_actual_state(runtime.host_index).boss;
    }
    
    // Determine whether we should save this boss (BossFinder)
    bool should_save_new = false;
    if (!boss_slug.empty()) {
        should_save_new = runtime.actions.should_save_path(boss_slug);
        stream.log("Boss: " + boss_slug + ", should save: " + (should_save_new ? "Yes" : "No"), COLOR_BLUE);
    }
    
    // Decide what to do with a followed path in Standard or StrongBoss
    bool keep_followed = followed_path ? runtime.actions.should_keep_followed_path() : false;
    if (followed_path) {
        stream.log("Followed path: " + std::string(keep_followed ? "keep" : "discard"), COLOR_BLUE);
    }
    
    // Overlay box to detect when a dialogue box is present (grey area at bottom)
    
    OverlayBoxScope dialog_box(stream.overlay(), {0.782, 0.850, 0.030, 0.050});
    
    // First step, press A after the initial greeting
    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    context.wait_for_all_requests();
    
    // Then, if there is something to be saved, check if the next dialogue box allows us to directly save or not (in case we want to save the path)
    
    if (followed_path) {
        stream.log("Handling followed-path dialogue.", COLOR_BLUE);
        // Keep the path or not
        if (keep_followed) {
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
            ImageStats stats = image_stats(extract_box_reference(screen, dialog_box));
            
            if (!is_grey(stats, 400, 1000)) {
                break; // Dialogue box gone
            }
            pbf_press_button(context, BUTTON_A, 160ms, 0ms);
            context.wait_for_all_requests();
            
        }
    }
    
    // Check if we have a new path to save
    
    if (!boss_slug.empty() && should_save_new) {
        stream.log("Checking for new-path saving dialogue", COLOR_BLUE);
        
        Language language = runtime.console_settings[console_index].language;
        
        size_t attempts = 0;
        const size_t MAX_ATTEMPTS = 20;
        bool done = false;
        while (!done && attempts < MAX_ATTEMPTS) {
            attempts++;
            context.wait_for(400ms);
            VideoSnapshot screen = stream.video().snapshot();
            if (!screen) continue;
            
            // Detect whether we are in the list to replace the Boss' paths or not
            
            std::vector<std::string> names = read_saved_paths(stream, language, screen);
            int non_empty = 0;
            for (const auto& n : names) {
                if (!n.empty()) ++non_empty;
            }
            bool in_list = (non_empty >= 2); // If there are at least 2 names visible, then there is a list to replace
            
            if (in_list) {
                stream.log("Detected replacement list.", COLOR_BLUE);
                int slot = find_unprotected_slot(names, runtime.actions, stream.logger());
                if (slot == -1) {
                    stream.log("Unable to save new boss, cancelling", COLOR_ORANGE);
                    pbf_press_button(context, BUTTON_B, 160ms, 0ms);
                } else {
                    for (int i = 0; i < slot; ++i) {
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 0ms);
                    };
                    context.wait_for_all_requests();
                    pbf_press_button(context, BUTTON_A, 160ms, 0ms);
                }
                
            } else {
                // The player didn't yet save 3 paths so we are free to save a path or not
                stream.log("Detected Yes/No prompt or simple message.", COLOR_BLUE);
                if (should_save_new) {
                    pbf_press_button(context, BUTTON_A, 160ms, 0ms);
                } else {
                    pbf_press_button(context, BUTTON_B, 160ms, 0ms);
                }
            }
            context.wait_for_all_requests();
            
            VideoSnapshot new_screen = stream.video().snapshot();
            if (!new_screen) continue;
            ImageStats stats = image_stats(extract_box_reference(new_screen, dialog_box));
            if (!is_grey(stats, 400, 1000)) {
                stream.log("New-path dialogue ended.", COLOR_BLUE);
                done = true;
            }
        }
        if (attempts >= MAX_ATTEMPTS) {
            stream.log("New-path save dialogue timed out.", COLOR_RED);
        }
    }
}



}
}
}
}
