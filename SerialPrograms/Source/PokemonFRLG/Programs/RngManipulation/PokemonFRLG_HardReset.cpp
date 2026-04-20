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

void rng_start_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t game_slot,
    uint8_t user_slot
){
    context.wait_for_all_requests();
    {
        HomeMenuWatcher detector(console);
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10000ms);
            },
            { detector }
        );
        if (ret == 0){
            console.log("Detected Home screen.");
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): Failed to detect Home screen after 10 seconds.",
                console
            );
        }
        context.wait_for(std::chrono::milliseconds(100));
    }

    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            ssf_press_dpad_ptv(context, DPAD_RIGHT, 160ms);
        }
        context.wait_for_all_requests();
    }

    pbf_press_button(context, BUTTON_A, 160ms, 340ms);

    WallClock deadline = current_time() + std::chrono::minutes(5);
    while (current_time() < deadline){
        HomeMenuWatcher home(console, std::chrono::milliseconds(2000));
        StartGameUserSelectWatcher user_select(console, COLOR_GREEN);
        UpdateMenuWatcher update_menu(console, COLOR_PURPLE);
        CheckOnlineWatcher check_online(COLOR_CYAN);
        FailedToConnectWatcher failed_to_connect(COLOR_YELLOW);
        BlackScreenWatcher black_screen(COLOR_BLUE, {0.1, 0.15, 0.8, 0.7});

        // spend a little bit longer waiting for the black screen to avoid missing it
        context.wait_for_all_requests();
        int ret1 = wait_until(
            console, context,
            std::chrono::seconds(2),
            { black_screen }
        );

        switch (ret1){
        case 0:
            console.log("Detected black screen. Game started...");
            return;
        default:
            console.log("Black screen not detected. Checking for other states...");
        }

        // handle other states
        context.wait_for_all_requests();
        int ret2 = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                home,
                user_select,
                update_menu,
                check_online,
                failed_to_connect,
                black_screen,
            }
        );

        //  Wait for screen to stabilize.
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret2){
        case 0:
            console.log("Detected home screen (again).", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 1:
            console.log("Detected user-select screen.");
            move_to_user(context, user_slot);
            pbf_press_button(context, BUTTON_A, 160ms, 320ms);
            break;
        case 2:
            console.log("Detected update menu.", COLOR_BLUE);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 3:
            console.log("Detected check online.", COLOR_BLUE);
            context.wait_for(std::chrono::seconds(1));
            break;
        case 4:
            console.log("Detected failed to connect.", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 5:
            console.log("Detected black screen. Game started...");
            return;
        default:
            console.log("start_game_from_home_with_inference(): No recognizable state after 30 seconds.", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 160ms, 840ms);
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "rng_start_game_from_home(): Failed to start game after 5 minutes.",
        console
    );
}


void reset_and_perform_blind_sequence(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    SeedButton SEED_BUTTON,
    uint64_t SEED_DELAY, 
    uint64_t CONTINUE_SCREEN_DELAY, 
    uint64_t TEACHY_DELAY, 
    uint64_t INGAME_DELAY, 
    bool SAFARI_ZONE,
    uint8_t PROFILE
){
    // close the game
    go_home(console, context);
    close_game_from_home(console, context);
    // start the game and quickly go back home
    rng_start_game_from_home(console, context, uint8_t(0), PROFILE); 
    pbf_wait(context, 200ms); // wait a moment to ensure the game doesn't fail to launch
    go_home(console, context);

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
            [TARGET, SEED_BUTTON, SEED_DELAY, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE](ProControllerContext& context) {
                perform_blind_sequence(context, TARGET, SEED_BUTTON, SEED_DELAY, CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, SAFARI_ZONE);
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

void reset_and_detect_copyright_text(ConsoleHandle& console, ProControllerContext& context, uint8_t PROFILE){
    go_home(console, context);
    close_game_from_home(console, context);
    rng_start_game_from_home(console, context, uint8_t(0), PROFILE);
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

}
}
}
