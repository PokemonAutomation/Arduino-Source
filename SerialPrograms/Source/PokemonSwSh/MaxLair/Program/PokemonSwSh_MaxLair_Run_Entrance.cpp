/*  Max Lair Run Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh_MaxLair_Run_Entrance.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"

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
    
    // Selection arrow in the path-list area — present when the "list is full / replace?" dialog is active
    SelectionArrowFinder top_region(stream.overlay(), {0.63, 0.50, 0.22, 0.10});

    // Selection arrow in the Yes/No area — present when the "save this path?" dialog is active
    SelectionArrowFinder yes_no_arrow(stream.overlay(), {0.63, 0.60, 0.22, 0.19});

    // White text check for the path-list entries — confirms the "list full" dialog
    OverlayBoxScope paths_box(stream.overlay(), {0.685, 0.515, 0.13, 0.013});

    // Grey dialog box — used to detect that any NPC dialog is still on screen
    OverlayBoxScope dialog_box(stream.overlay(), {0.78, 0.85, 0.03, 0.05});

    // Timeout: 5 minutes
    auto start_time = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::minutes(5);
    
    while(true){
        auto now = std::chrono::steady_clock::now();
        if (now - start_time > timeout){
            stream.log("Entrance dialogue timed out after 5 minutes.", COLOR_RED);
            throw OperationFailedException(ErrorReport::SEND_ERROR_REPORT, "Entrance dialogue timed out.", stream);
        }
        
        context.wait_for_all_requests();

        context.wait_for(1000ms);
        VideoSnapshot screen = stream.video().snapshot();
        if (!screen) continue;
        
        bool top_arrow_found = top_region.detect(screen);
        bool bottom_arrow_found = yes_no_arrow.detect(screen);

        ImageStats paths_box_stats = image_stats(extract_box_reference(screen, paths_box));
        bool paths_box_present = is_white(paths_box_stats, 400, 10);

        ImageStats dialog_box_stats = image_stats(extract_box_reference(screen, dialog_box));
        bool dialog_box_present = is_grey(dialog_box_stats, 400, 1000);

        if (top_arrow_found && paths_box_present) {
            
            if (save_path){
                // List of bosses is full, stop the program
                stream.log("Cannot save path – saved list is full. Stopping program.", COLOR_RED);
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    "Paths list is full. Program stopped.",
                    stream
                );
                
            }else{
                stream.log("Not saving path");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
            
        } else if (bottom_arrow_found) {
            
            if (save_path){
                if (followed_path){
                    stream.log("Keeping old path.");
                    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                }else{
                    std::string display_name = get_pokemon_name(boss_slug).display_name();
                    stream.log("Saving new path");
                    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                    send_program_notification(
                        env,
                        runtime.notification_status,
                        COLOR_BLUE,
                        "Path Saved",
                        {{"Boss: ", display_name}},
                        ""
                    );
                }
            }else{
                stream.log("Not saving new path");
                pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            }
        }else if (!dialog_box_present){
            return;
        }
        
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
    }
}



}
}
}
}
