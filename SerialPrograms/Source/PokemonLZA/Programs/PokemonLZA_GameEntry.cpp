/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonLZA/PokemonLZA_Settings.h"
#include "PokemonLZA_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

bool reset_game_to_gamemenu(
    ConsoleHandle& console, ProControllerContext& context
){
    from_home_close_and_reopen_game(console, context, true);

    // Now the game has opened:
    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT);
}

//  From the game menu screen (where "Press A" is displayed to enter the game),
//  mash A to enter the game and wait until the black screen is gone.
bool gamemenu_to_ingame(
    VideoStream& stream, ProControllerContext& context,
    Milliseconds enter_game_timeout
){
    stream.log("Mashing A to enter game...");
    {
        BlackScreenWatcher detector(COLOR_RED, {0.1, 0.04, 0.8, 0.3});
        stream.log("Waiting to enter game...");
        int ret = run_until<ProControllerContext>(
            stream, context,
            [=](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_A, 2s);
                pbf_wait(context, enter_game_timeout);
            },
            {{detector}}
        );
        if (ret != 0){
            stream.log("Timed out waiting for black screen.", COLOR_RED);
            return false;
        }
    }

    stream.log("Black screen detected");

    {
        BlackScreenOverWatcher detector(COLOR_RED, {0.1, 0.04, 0.8, 0.3});
        int ret = wait_until(
            stream, context,
            std::chrono::milliseconds(enter_game_timeout),
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
}

bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool backup_save,
    Milliseconds enter_game_timeout,
    Milliseconds post_wait_time
){
    console.log("Resetting game from Home...");
    console.overlay().add_log("Resetting game...");
    bool ok = true;
    ok &= reset_game_to_gamemenu(console, context);

    if (backup_save){
        console.log("Loading backup save!");
        console.overlay().add_log("Use Backup Save");
        pbf_wait(context, 1000ms);
        ssf_press_dpad(context, DPAD_UP, 0ms, 200ms);
        ssf_press_button(context, BUTTON_B | BUTTON_X, 1000ms, 200ms);
    }

    ok &= gamemenu_to_ingame(
        console, context,
        enter_game_timeout
    );
    if (!ok){
        dump_image(console.logger(), env.program_info(), console.video(), "StartGame");
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}
bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool backup_save,
    Milliseconds post_wait_time
){
    return reset_game_from_home(
        env, console, context,
        backup_save,
        GameSettings::instance().ENTER_GAME_WAIT,
        post_wait_time
    );
}


}
}
}
