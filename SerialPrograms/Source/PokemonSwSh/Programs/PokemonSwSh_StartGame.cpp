/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_StartGame.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_loading_game(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool backup_save,
    uint16_t post_wait_time
){
    openedgame_to_gamemenu(env, console, context, GameSettings::instance().START_GAME_WAIT);

    env.log("enter_loading_game(): Game Loaded. Entering game...", COLOR_PURPLE);
    enter_game(context, backup_save, GameSettings::instance().ENTER_GAME_MASH, 0);
    context.wait_for_all_requests();

    //  Wait to enter game.
    {
        std::chrono::milliseconds timeout(GameSettings::instance().ENTER_GAME_WAIT * (1000 / TICKS_PER_SECOND));

        InferenceBoxScope box(console, 0.2, 0.2, 0.6, 0.6);

        bool black_found = false;

        InferenceThrottler throttler(timeout);
        while (true){
            env.check_stopping();

            QImage screen = console.video().snapshot();
            if (screen.isNull()){
                env.log("enter_loading_game(): Screenshot failed.", COLOR_PURPLE);
                throttler.set_period(std::chrono::milliseconds(1000));
            }else{
                bool black = is_black(extract_box_reference(screen, box));
                if (black){
                    if (!black_found){
                        env.log("enter_loading_game(): Game entry started.", COLOR_PURPLE);
                    }
                    black_found = true;
                }else if (black_found){
                    break;
                }
            }

            if (throttler.end_iteration(env)){
                env.log("enter_loading_game(): Game entry timed out. Proceeding with default start delay.", COLOR_RED);
                break;
            }
        }
    }
    env.log("start_game_with_inference(): Game started.", COLOR_PURPLE);

    if (post_wait_time != 0){
        pbf_wait(context, post_wait_time);
    }
}

void start_game_from_home_with_inference(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save,
    uint16_t post_wait_time
){
    open_game_from_home(
        env, console, context,
        tolerate_update_menu,
        game_slot,
        user_slot,
        GameSettings::instance().START_GAME_MASH
    );

    //  Wait for game to load.
    enter_loading_game(env, console, context, backup_save, post_wait_time);
}

void reset_game_from_home_with_inference(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    bool backup_save,
    uint16_t post_wait_time
){
    if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game(context);
        start_game_from_home_with_inference(
            env, console, context, tolerate_update_menu, 0, 0, backup_save, post_wait_time
        );
        return;
    }

    fast_reset_game(context, GameSettings::instance().START_GAME_MASH, 0, 0, 0);
    context.wait_for_all_requests();

    //  Wait for game to load.
    enter_loading_game(env, console, context, backup_save, post_wait_time);
}




}
}
}
