/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh_StartGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_loading_game(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool backup_save,
    uint16_t post_wait_time
){
    //  Wait for game to load.
    {
        std::chrono::milliseconds timeout(START_GAME_WAIT * (1000 / TICKS_PER_SECOND));

        InferenceBoxScope box0(console, 0.2, 0.2, 0.6, 0.1);
        InferenceBoxScope box1(console, 0.2, 0.7, 0.6, 0.1);

        bool black_found = false;

        InferenceThrottler throttler(timeout);
        while (true){
            env.check_stopping();

            QImage screen = console.video().snapshot();
            if (screen.isNull()){
                env.log("enter_loading_game(): Screenshot failed.", "purple");
                throttler.set_period(std::chrono::milliseconds(1000));
            }else{
                bool black0 = is_black(extract_box(screen, box0));
                bool black1 = is_black(extract_box(screen, box1));
                if (black0 && black1){
                    if (!black_found){
                        env.log("start_game_with_inference(): Game load started.", "purple");
                    }
                    black_found = true;
                }else if (black_found){
                    break;
                }
            }

            if (throttler.end_iteration(env)){
                env.log("enter_loading_game(): Game load timed out. Proceeding with default start delay.", "red");
                break;
            }
        }
    }

    env.log("enter_loading_game(): Game Loaded. Entering game...", "purple");
    enter_game(console, backup_save, ENTER_GAME_MASH, 0);
    console.botbase().wait_for_all_requests();

    //  Wait to enter game.
    {
        std::chrono::milliseconds timeout(ENTER_GAME_WAIT * (1000 / TICKS_PER_SECOND));

        InferenceBoxScope box(console, 0.2, 0.2, 0.6, 0.6);

        bool black_found = false;

        InferenceThrottler throttler(timeout);
        while (true){
            env.check_stopping();

            QImage screen = console.video().snapshot();
            if (screen.isNull()){
                env.log("enter_loading_game(): Screenshot failed.", "purple");
                throttler.set_period(std::chrono::milliseconds(1000));
            }else{
                bool black = is_black(extract_box(screen, box));
                if (black){
                    if (!black_found){
                        env.log("enter_loading_game(): Game entry started.", "purple");
                    }
                    black_found = true;
                }else if (black_found){
                    break;
                }
            }

            if (throttler.end_iteration(env)){
                env.log("enter_loading_game(): Game entry timed out. Proceeding with default start delay.", "red");
                break;
            }
        }
    }
    env.log("start_game_with_inference(): Game started.", "purple");

    if (post_wait_time != 0){
        pbf_wait(console, post_wait_time);
    }
}

void start_game_from_home_with_inference(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save,
    uint16_t post_wait_time
){
    if (game_slot != 0){
        pbf_press_button(console, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(console, DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(console, BUTTON_A, 5, 35);      //  Choose game
        pbf_press_dpad(console, DPAD_UP, 5, 0);          //  Skip the update window.
    }

    if (!START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(console, BUTTON_A, START_GAME_MASH);
    }else{
        pbf_press_button(console, BUTTON_A, 5, 175);     //  Enter select user menu.
        if (user_slot != 0){
            //  Move to correct user.
            for (uint8_t c = 0; c < 8; c++){
                pbf_press_dpad(console, DPAD_LEFT, 7, 7);
            }
//            pbf_wait(50);
            for (uint8_t c = 1; c < user_slot; c++){
                pbf_press_dpad(console, DPAD_RIGHT, 7, 7);
            }
        }
        pbf_press_button(console, BUTTON_A, 5, 5);       //  Enter game

        //  Switch to mashing ZR instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = START_GAME_MASH;
        if (START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(console, BUTTON_ZR, duration);
    }
    console.botbase().wait_for_all_requests();

    //  Wait for game to load.
    enter_loading_game(env, console, backup_save, post_wait_time);
}

void reset_game_from_home_with_inference(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    if (START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game(console);
        start_game_from_home_with_inference(
            env, console, tolerate_update_menu, 0, 0, false, post_wait_time
        );
        return;
    }

    fast_reset_game(console, START_GAME_MASH, 0, 0, 0);
    console.botbase().wait_for_all_requests();

    //  Wait for game to load.
    enter_loading_game(env, console, false, post_wait_time);
}




}
}
}
