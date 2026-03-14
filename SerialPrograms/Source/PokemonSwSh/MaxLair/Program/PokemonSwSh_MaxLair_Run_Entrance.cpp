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
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"

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
        auto cropped = extract_box_reference(
            screen,
            NAME_BOXES[i]
        );
        OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
                stream.logger(),
                language, cropped,
                OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
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
        if (!actions.is_in_save_list(current_slugs[i])) {
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
    context.wait_for(1000ms);
    
    // Get the boss slug
    std::string boss_slug;
    if (runtime.host_index < runtime.console_settings.active_consoles()) {
        boss_slug = state_tracker.infer_actual_state(runtime.host_index).boss;
    };
    
    bool save_path = false;
    
    if (!followed_path) {
        // Check if the user checked the box to save the path when running the BossFinder program
        
        if (!boss_slug.empty()) {
            save_path = runtime.actions.is_in_save_list(boss_slug);
            stream.log("Boss: " + boss_slug + ", should save: " + (save_path ? "Yes" : "No"), COLOR_BLUE);
        };
    } else {
        save_path = followed_path;
    }
    
    Language language = runtime.console_settings[console_index].language;
    
    // Overlay box to detect when a dialogue box, a Yes/No option to save a path or erasing a path if our list is full is present
    OverlayBoxScope dialog_box(stream.overlay(), {0.78, 0.85, 0.03, 0.05});
    OverlayBoxScope yes_no_box(stream.overlay(), {0.68, 0.75, 0.135, 0.02});
    OverlayBoxScope paths_box(stream.overlay(), {0.685, 0.515, 0.13, 0.013});
    
    // Timeout: 5 minutes
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::minutes(5);
    
    while(true) {
        auto now = std::chrono::steady_clock::now();
        if (now - start_time > timeout) {
            stream.log("Entrance dialogue timed out after 5 minutes.", COLOR_RED);
            throw OperationFailedException(ErrorReport::SEND_ERROR_REPORT, "Entrance dialogue timed out.", stream);
        }
        
        context.wait_for_all_requests();
        context.wait_for(2000ms);
        VideoSnapshot screen = stream.video().snapshot();
        if (!screen) continue;
        
        ImageStats dialog_box_stats = image_stats(extract_box_reference(screen, dialog_box));
        
        ImageStats yes_no_box_stats = image_stats(extract_box_reference(screen, yes_no_box));
        
        ImageStats paths_box_stats = image_stats(extract_box_reference(screen, paths_box));
        
        bool dialog_box_present = is_grey(dialog_box_stats, 400, 1000);
        
        bool yes_no_box_present = is_white(yes_no_box_stats, 400, 10);
        
        bool paths_box_present = is_white(paths_box_stats, 400, 10);
        
        if (paths_box_present && yes_no_box_present) {
            
            if (save_path) {
                // Bring the cursor to the bottom position to correctly identify boss names
                pbf_press_dpad(context, DPAD_UP, 160ms, 80ms);
                
                context.wait_for_all_requests();
                
                VideoSnapshot clean_screen = stream.video().snapshot();
                
                if (!clean_screen) continue;
                
                size_t attempts = 0;
                const size_t MAX_ATTEMPTS = 10;
                bool done = false;
                while (!done && attempts < MAX_ATTEMPTS) {
                    attempts++;
                    
                    std::vector<std::string> names = read_saved_paths(stream, language, clean_screen);
                    int non_empty = 0;
                    for (const auto& n : names) {
                        if (!n.empty()) ++non_empty;
                    }
                    bool in_list = (non_empty >= 2); // If there are at least 2 readable names, then we can read the list
                    
                    if (in_list) {
                        context.wait_for(1000ms);
                        
                        int slot = find_unprotected_slot(names, runtime.actions, stream.logger());
                        if (slot == -1) {
                            stream.log("Unable to save new boss or all bosses from list already saved, cancelling", COLOR_ORANGE);
                            runtime.session_stats.add_error();
                            pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
                        } else {
                            // Bring cursor back to the top position
                            pbf_press_dpad(context, DPAD_DOWN, 160ms, 500ms);
                            context.wait_for(1000ms);
                            // Then move down to target slot
                            for (int i = 0; i < slot; ++i) {
                                pbf_press_dpad(context, DPAD_DOWN, 160ms, 500ms);
                            };
                            context.wait_for_all_requests();
                            stream.log("Erasing old path and saving new path");
                            pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                            
                            send_program_notification(env, runtime.notification_status, COLOR_BLUE, "Path Saved", {{"Boss: ", boss_slug}}, "");
                            
                        }
                        context.wait_for_all_requests();
                        done = true;
                    }
                    if (attempts >= MAX_ATTEMPTS) {
                        stream.log("New-path save dialogue timed out.", COLOR_RED);
                    };
                }
            } else {
                stream.log("Not saving path");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            };
            
        } else if (!paths_box_present && yes_no_box_present) {
            
            if (save_path) {
                stream.log("Saving new path or keeping old path");
                pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                send_program_notification(env, runtime.notification_status, COLOR_BLUE, "Path Saved", {{"Boss: ", boss_slug}}, "");
            } else {
                stream.log("Not saving new path");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
        } else if (!dialog_box_present) {
            return;
        };
        
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    }
    
    
    
}
}
}
}
}
