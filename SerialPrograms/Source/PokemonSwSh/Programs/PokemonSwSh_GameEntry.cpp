/*  Start Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



void resume_game_no_interact(
    ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu
){
    bool video_available = console.video().snapshot();
    if (video_available){
        resume_game_from_home(console, context);
    }else{
        resume_game_no_interact_old(context, tolerate_update_menu);
    }
}
void resume_game_back_out(
    ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu, uint16_t mash_B_time
){
    bool video_available = console.video().snapshot();
    if (video_available){
        resume_game_from_home(console, context);
    }else{
        resume_game_back_out_old(context, tolerate_update_menu, mash_B_time);
    }
}




void enter_loading_game(
    ConsoleHandle& console, BotBaseContext& context,
    bool backup_save,
    uint16_t post_wait_time
){
    openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT);

    console.log("enter_loading_game(): Game Loaded. Entering game...", COLOR_PURPLE);
    enter_game(context, backup_save, GameSettings::instance().ENTER_GAME_MASH, 0);
    context.wait_for_all_requests();

    //  Wait to enter game.
    {
        std::chrono::milliseconds timeout(GameSettings::instance().ENTER_GAME_WAIT * (1000 / TICKS_PER_SECOND));

        OverlayBoxScope box(console, {0.2, 0.2, 0.6, 0.6});

        bool black_found = false;

        InferenceThrottler throttler(timeout);
        while (true){
            context.throw_if_cancelled();

            std::shared_ptr<const ImageRGB32> screen = console.video().snapshot();
            if (!screen){
                console.log("enter_loading_game(): Screenshot failed.", COLOR_PURPLE);
                throttler.set_period(std::chrono::milliseconds(1000));
            }else{
                bool black = is_black(extract_box_reference(*screen, box));
                if (black){
                    if (!black_found){
                        console.log("enter_loading_game(): Game entry started.", COLOR_PURPLE);
                    }
                    black_found = true;
                }else if (black_found){
                    break;
                }
            }

            if (throttler.end_iteration(context)){
                console.log("enter_loading_game(): Game entry timed out. Proceeding with default start delay.", COLOR_RED);
                break;
            }
        }
    }
    console.log("start_game_with_inference(): Game started.", COLOR_PURPLE);

    if (post_wait_time != 0){
        pbf_wait(context, post_wait_time);
    }
}

void start_game_from_home_with_inference(
    ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    bool backup_save,
    uint16_t post_wait_time
){
    NintendoSwitch::start_game_from_home(
        console,
        context,
        tolerate_update_menu,
        game_slot,
        user_slot,
        GameSettings::instance().START_GAME_MASH
    );

    //  Wait for game to load.
    enter_loading_game(console, context, backup_save, post_wait_time);
}

void reset_game_from_home_with_inference(
    ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    bool backup_save,
    uint16_t post_wait_time
){
    bool video_available = console.video().snapshot();
    if (video_available ||
        ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET ||
        tolerate_update_menu
    ){
        close_game(context);
        start_game_from_home_with_inference(
            console, context, tolerate_update_menu, 0, 0, backup_save, post_wait_time
        );
        return;
    }

    fast_reset_game(context, GameSettings::instance().START_GAME_MASH, 0, 0, 0);
    context.wait_for_all_requests();

    //  Wait for game to load.
    enter_loading_game(console, context, backup_save, post_wait_time);
}




}
}
}
