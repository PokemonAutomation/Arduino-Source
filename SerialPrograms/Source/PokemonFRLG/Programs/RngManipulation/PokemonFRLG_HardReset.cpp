/*  Hard Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Inference/NintendoSwitch_CheckOnlineDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_FailedToConnectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_CloseGameDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_HardReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

void rng_reset_and_return_home(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t user_slot
){
    // close the game
    go_home(console, context);
    close_game_from_home(console, context);

    // console specific delays between opening the game and returning to the Home screen
    ConsoleType console_type = console.state().console_type(); // go_home() has already been called, so this should already be detected
    Milliseconds launch_delay = (console_type == ConsoleType::Switch1) ? 450ms : 950ms;

    bool extra_resume_press = false;
    WallClock deadline = current_time() + std::chrono::minutes(5);
    while (true){
        if (current_time() > deadline){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "rng_start_game_and_return_home(): Failed to start game and return to Home within 5 minutes.",
                console
            );
        }

        StartGameUserSelectWatcher user_select(console, COLOR_GREEN);
        HomeMenuWatcher home(console, std::chrono::milliseconds(2000));
        UpdateMenuWatcher update_menu(console, COLOR_PURPLE);
        CheckOnlineWatcher check_online(COLOR_CYAN);
        FailedToConnectWatcher failed_to_connect(COLOR_YELLOW);

        // first, get to the user select screen
        context.wait_for_all_requests();
        int ret2 = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                user_select,
                home,
                update_menu,
                check_online,
                failed_to_connect,
            }
        );

        //  Wait for screen to stabilize.
        context.wait_for(std::chrono::milliseconds(100));

        // some of these states might not be relevant at this point (if they would only show up after pressing A on a profile)
        switch (ret2){
        case 0:
            console.log("Detected user-select screen.");
            move_to_user(context, user_slot);
            break;
        case 1:
            console.log("Detected Home screen.", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);            
            continue;
        case 2:
            console.log("Detected update menu.", COLOR_BLUE);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);     
            continue;
        case 3:
            console.log("Detected check online.", COLOR_BLUE);
            context.wait_for(std::chrono::seconds(1));
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);     
            continue;
        case 4:
            console.log("Detected failed to connect.", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            continue;
        default:
            console.log("rng_start_game_and_return_home(): No recognizable state after 30 seconds.", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 160ms, 840ms);
            continue;
        }

        context.wait_for_all_requests();

        // By this point, the user selection menu is open with the desired profile selected, and the game hasn't yet been started.
        // Everything up to this point has not been time-sensitive.
        // Waiting for all requests and using inference should be avoided for any button presses that happen *while the game is open*, 
        // but we can make sure we've gotten back to the home screen and pause there

        // open the game and go back home ASAP
        if (extra_resume_press){
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        }
        pbf_press_button(context, BUTTON_A, 50ms, launch_delay);
        pbf_press_button(context, BUTTON_HOME, 200ms, 1800ms);

        while(current_time() < deadline){
            // make sure a black screen appeared as a result of the button presses
            // if a popup appears, flag that it happened even if it gets closed by the home button press
            // Not sure how to handle the online check, so leaving it out for now
            BlackScreenWatcher black_screen(COLOR_BLUE, {0.1, 0.15, 0.8, 0.7});
            int ret3 = wait_until(
                console, context,
                std::chrono::seconds(2),
                { black_screen, update_menu, failed_to_connect },
                1ms
            );
            bool black_screen_detected = false;
            switch (ret3){
            case 0:
                console.log("Black screen detected.");
                black_screen_detected = true;
                break;
            case 1:
                console.log("Update menu detected.", COLOR_BLUE);
                extra_resume_press = true;
            case 2:
                console.log("Failed to connect detected.", COLOR_BLUE);
                extra_resume_press = true;
            }

            context.wait_for_all_requests();

            // make sure we're back at the home screen. Otherwise, handle any undesired states
            int ret4 = wait_until(
                console, context,
                std::chrono::seconds(10),
                { 
                    home, 
                    user_select,
                    update_menu,
                    failed_to_connect 
                }
            );

            switch (ret4){
            case 0:
                if (black_screen_detected){
                    console.log("Detected Home screen after black screen.");
                    return;
                }else{
                    console.log("Detected Home screen, but no black screen. Trying again from the beginning...", COLOR_BLUE);
                    launch_delay += 250ms; // bump this up in case it was too short
                    break; // back to the outer loop
                }
                return;
            case 1:
                console.log("Detected user-select screen. Trying again...", COLOR_BLUE);
                move_to_user(context, user_slot);
                pbf_press_button(context, BUTTON_A, 50ms, launch_delay);
                pbf_press_button(context, BUTTON_HOME, 200ms, 2800ms);
                continue;
            case 2:
                console.log("Detected update menu. Trying again...", COLOR_BLUE);
                pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
                pbf_press_button(context, BUTTON_A, 50ms, launch_delay);
                pbf_press_button(context, BUTTON_HOME, 200ms, 2800ms);
                continue;
            case 3:
                console.log("Detected failed to connect.", COLOR_BLUE);
                pbf_press_button(context, BUTTON_A, 50ms, launch_delay);
                pbf_press_button(context, BUTTON_HOME, 200ms, 2800ms);
                continue;
            default:
                console.log("rng_start_game_and_return_home(): No recognizable state after 10 seconds.", COLOR_RED);
                pbf_press_button(context, BUTTON_HOME, 160ms, 840ms);
                break; // back to outer loop
            }

            break;
        }
    }
}


void reset_and_perform_blind_sequence(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    SeedButton SEED_BUTTON,
    BlackoutButton BLACKOUT_BUTTON,
    uint64_t SEED_DELAY, 
    uint64_t CONTINUE_SCREEN_DELAY, 
    uint64_t TEACHY_DELAY, 
    uint64_t INGAME_DELAY, 
    bool SAFARI_ZONE,
    uint8_t PROFILE
){
    rng_reset_and_return_home(console, context, PROFILE); 

    // attempt to resume the game and perform the blind sequence
    // by this point, the license check should be over, so we don't need to worry about it when resuming the game
    uint8_t attempts = 0;
    while(true){
        if (attempts >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "RngHelper(): Failed to reset the game 5 times in a row.",
                console
            );  
        }
        console.log("Starting blind button presses...");
        UpdateMenuWatcher update_detector(console);
        StartGameUserSelectWatcher user_selection_detector(console);
        // any other fail conditions should be added here
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [TARGET, SEED_BUTTON, BLACKOUT_BUTTON, SEED_DELAY, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE](ProControllerContext& context) {
                perform_blind_sequence(context, TARGET, SEED_BUTTON, BLACKOUT_BUTTON, SEED_DELAY, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE);
            },
            { update_detector, user_selection_detector },
            1000ms
        );

        switch (ret){
        case 0: 
            attempts++;
            console.log("Detected update window.", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 80ms, 4000ms);
            context.wait_for_all_requests();
            continue;
        case 1:
            attempts++;
            console.log("Detected the user selection screen. Reattempting to start the game");
            pbf_press_button(context, BUTTON_A, 160ms, 1040ms);
            go_home(console, context);
            continue;
        default:
            return;
        }
    }
}

#if 0
void reset_and_detect_copyright_text(ConsoleHandle& console, ProControllerContext& context, uint8_t PROFILE){
    go_home(console, context);
    close_game_from_home(console, context);
    rng_start_game_and_return_home(console, context, uint8_t(0), PROFILE);
    pbf_wait(context, 200ms); // add an extra delay to try to ensure the game doesn't fail to launch
    go_home(console, context);

    uint8_t attempts = 0;
    while(true){
        if (attempts >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to resume the game 5 times in a row.",
                console
            );  
        }

        UpdateMenuWatcher update_detector(console);
        StartGameUserSelectWatcher user_selection_detector(console);
        BlackScreenWatcher blackscreen_detector(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 80ms, 9920ms);
            },
            { update_detector, user_selection_detector, blackscreen_detector }
        );

        BlackScreenOverWatcher copyright_detector(COLOR_RED);
        int ret2;
        switch (ret){
        case 0: 
            attempts++;
            console.log("Detected update window.", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 80ms, 4000ms);
            context.wait_for_all_requests();
            continue;
        case 1:
            attempts++;
            console.log("Detected the user selection screen. Reattempting to start the game");
            pbf_press_button(context, BUTTON_A, 160ms, 1040ms);
            go_home(console, context);
            continue;
        case 2: 
            context.wait_for_all_requests();
            ret2 = wait_until(
                console, context, 10000ms,
                {copyright_detector },
                1ms // catch black screen as quickly as possible
            );
            if (ret2 < 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Black screen detected for more than 10 seconds after starting game.",
                    console
                );
            }
            return;
        default:
            console.log("No black screen or update popup detected. Pressing A again...");
            continue;
        }
    }

}
#endif

}
}
}
