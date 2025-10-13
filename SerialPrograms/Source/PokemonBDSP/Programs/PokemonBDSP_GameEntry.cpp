/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


bool gamemenu_to_ingame(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
){
    stream.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(context, BUTTON_ZL, mash_duration);
    context.wait_for_all_requests();
    stream.log("Waiting to enter game...");
    int ret = wait_until(
        stream, context,
        enter_game_timeout,
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
bool openedgame_to_ingame(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    Milliseconds load_game_timeout,
    Milliseconds mash_duration, Milliseconds enter_game_timeout,
    Milliseconds post_wait_time
){
    bool ok = true;
    ok &= openedgame_to_gamemenu(stream, context, load_game_timeout);
    ok &= gamemenu_to_ingame(stream, context, mash_duration, enter_game_timeout);
    if (!ok){
        dump_image(stream.logger(), env.program_info(), stream.video(), "StartGame");
    }
    stream.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}




bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    Milliseconds post_wait_time
){
    from_home_close_and_reopen_game(console, context, true);

    bool ret = openedgame_to_ingame(
        env, console, context,
        GameSettings::instance().START_GAME_WAIT0,
        GameSettings::instance().ENTER_GAME_MASH0,
        GameSettings::instance().ENTER_GAME_WAIT0,
        post_wait_time
    );
    context.wait_for_all_requests();
    return ret;
}



}
}
}
