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
        {0.685000, 0.531000, 0.130000, 0.061000},
        {0.685000, 0.586000, 0.130000, 0.061000},
        {0.685000, 0.645000, 0.130000, 0.053000}
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
        should_save_new = runtime.actions.save_path(boss_slug);
        stream.log("Boss: " + boss_slug + ", should save: " + (should_save_new ? "Yes" : "No"), COLOR_BLUE);
    }
    
    // Overlay box to detect when a dialogue box is present (grey area at bottom)
    OverlayBoxScope dialog_box(stream.overlay(), {0.78, 0.85, 0.03, 0.05});
    OverlayBoxScope yes_no_box(stream.overlay(), {0.75, 0.72, 0.06, 0.05});
    
    // First step, press A after the initial greeting
    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    context.wait_for_all_requests();
    context.wait_for(400ms);
    
    // Check after the first A press what happens
    VideoSnapshot screen = stream.video().snapshot();
    if (!screen) return;
    // Check if there is a dialog box
    ImageStats stats = image_stats(extract_box_reference(screen, dialog_box));
    bool dialog_present = is_grey(stats, 400, 1000);
    
    if (!dialog_present) {
        stream.log("Path is already in the list");
        return; // Dialogue box gone
    } else {
        // Dialog present, is there a Yes/No box?
        ImageStats yes_no_stats = image_stats(extract_box_reference(screen, yes_no_box));
        bool yes_no_box_present = is_white(yes_no_stats, 400, 10);
        if (yes_no_box_present) {
            stream.log("Detected Yes/No box");
            // There is a Yes/No box, which means that we encountered a new boss and we are asked to save it or not
            if (save_path) {
                // The path was previously chosen so we need to keep it, or it is a new path and it was chosen from the list in BossFinder
                pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                stream.log("Path saved");
            } else {
                stream.log("Path discarded");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
            context.wait_for_all_requests();
            
            /// TODO: HANDLE DIALOG AFTER CHOICES HAVE BEEN MADE
            return;
            
        };
        
        // There is only a dialog box but no choice yet, first press A again to see if it makes a Yes/No box appear
        stream.log("No Yes/No box detected, only dialog");
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        context.wait_for_all_requests();
        context.wait_for(400ms);
        
        VideoSnapshot screen_two = stream.video().snapshot();
        if (!screen_two) return;
        
        yes_no_stats = image_stats(extract_box_reference(screen_two, yes_no_box));
        
        yes_no_box_present = is_white(yes_no_stats, 400, 10);
        
        if (yes_no_box_present) {
            stream.log("Detected Yes/No box");
            // There is now a Yes/No choice box, which means that we followed a path and we are asked if we want to keep it
            if (save_path) {
                // The path was previously chosen so we need to keep it, or it is a new path and it was chosen from the list in BossFinder
                stream.log("Path saved");
                pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
            } else {
                stream.log("Path discarded");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
            context.wait_for_all_requests();
            
            /// TODO: HANDLE DIALOG AFTER CHOICES HAVE BEEN MADE
            return;
        };
        
        // Press A again to check if we lost to the boss?
        stream.log("Checking if we lost to the previous boss?");
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        context.wait_for_all_requests();
        
        VideoSnapshot screen_three = stream.video().snapshot();
        if (!screen_three) return;
        
        stats = image_stats(extract_box_reference(screen_three, dialog_box));
        bool dialog_present = is_grey(stats, 400, 1000);
        
        if (!dialog_present) {
            // We lost to the boss, exiting this part of the loop;
            stream.log("We lost against the boss, entrance state cleared");
            return;
        }
        
        // Our list of paths is full, so we need to check if we need to save the path
        
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_dpad(context, DPAD_UP, 160ms, 80ms);
        
        context.wait_for_all_requests();
        
        Language language = runtime.console_settings[console_index].language;
        
        size_t attempts = 0;
        const size_t MAX_ATTEMPTS = 10;
        bool done = false;
        while (!done && attempts < MAX_ATTEMPTS) {
            attempts++;
            context.wait_for(400ms);
            VideoSnapshot screen_four = stream.video().snapshot();
            if (!screen_four) continue;
            
            std::vector<std::string> names = read_saved_paths(stream, language, screen_four);
            int non_empty = 0;
            for (const auto& n : names) {
                if (!n.empty()) ++non_empty;
            }
            bool in_list = (non_empty >= 2); // If there are at least 2 names visible, then there is a list to replace
            
            if (in_list) {
                stream.log("Detected replacement list.", COLOR_BLUE);
                context.wait_for(200ms);
                
                VideoSnapshot clean_screen = stream.video().snapshot();
                if (!clean_screen) continue;
                
                names = read_saved_paths(stream, language, clean_screen);
                
                int slot = find_unprotected_slot(names, runtime.actions, stream.logger());
                if (slot == -1) {
                    stream.log("Unable to save new boss, cancelling", COLOR_ORANGE);
                    pbf_press_button(context, BUTTON_B, 160ms, 0ms);
                } else {
                    // Bring cursor back to the top position
                    pbf_press_dpad(context, DPAD_DOWN, 160ms, 80ms);
                    // Then move down to target slot
                    for (int i = 0; i < slot; ++i) {
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 0ms);
                    };
                    context.wait_for_all_requests();
                    pbf_press_button(context, BUTTON_A, 160ms, 0ms);
                }
                context.wait_for_all_requests();
                done = true;
            }
            if (attempts >= MAX_ATTEMPTS) {
                stream.log("New-path save dialogue timed out.", COLOR_RED);
            }
        }
    }

    
    
}
}
}
