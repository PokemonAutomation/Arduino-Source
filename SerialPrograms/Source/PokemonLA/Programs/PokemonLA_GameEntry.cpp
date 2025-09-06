/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


bool reset_game_to_gamemenu(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu
){
    bool video_available = (bool)console.video().snapshot();
    if (video_available ||
        ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET ||
        tolerate_update_menu
    ){
        close_game_from_home(console, context);
        start_game_from_home(
            console,
            context,
            tolerate_update_menu,
            0, 0,
            GameSettings::instance().START_GAME_MASH0
        );
    }else{
        pbf_press_button(context, BUTTON_X, 50, 0);
        pbf_mash_button(context, BUTTON_A, GameSettings::instance().START_GAME_MASH0);
    }

    // Now the game has opened:
    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT1);
}

bool gamemenu_to_ingame(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
){
    stream.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(context, BUTTON_A, mash_duration);
    context.wait_for_all_requests();
    stream.log("Waiting to enter game...");
    int ret = wait_until(
        stream, context,
        std::chrono::milliseconds(enter_game_timeout * (1000 / TICKS_PER_SECOND)),
        {{detector}}
    );
    if (ret == 0){
        stream.log("Entered game!");
        return true;
    }else{
        stream.log("Timed out waiting to enter game.", COLOR_RED);
        return false;
    }
}

bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    bool ok = true;
    ok &= reset_game_to_gamemenu(console, context, tolerate_update_menu);
    ok &= gamemenu_to_ingame(
        console, context,
        GameSettings::instance().ENTER_GAME_MASH0,
        GameSettings::instance().ENTER_GAME_WAIT0
    );
    if (!ok){
        dump_image(console.logger(), env.program_info(), console.video(), "StartGame");
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}





}
}
}
