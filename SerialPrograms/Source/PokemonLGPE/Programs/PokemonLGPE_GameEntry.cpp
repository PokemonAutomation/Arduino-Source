/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "Controllers/ControllerTypes.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
//#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "PokemonLGPE/PokemonLGPE_Settings.h"
#include "PokemonLGPE_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{


bool reset_game_to_gamemenu(
    ConsoleHandle& console, JoyconContext& context
){
    close_game_from_home(console, context);
    start_game_from_home_with_inference(
        console,
        context,
        0, 0,
        GameSettings::instance().START_GAME_MASH0
    );

    // Now the game has opened:
    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT1);
}

bool gamemenu_to_ingame(
    VideoStream& stream, JoyconContext& context,
    Milliseconds mash_duration, Milliseconds enter_game_timeout
){
    //Includes choosing the controller.
    //Controllers are disconnected? on selection screen so make sure to mash.
    stream.log("Mashing A to enter game and select controller...");
    pbf_mash_button(context, BUTTON_A, mash_duration);
    context.wait_for_all_requests();

    //White screen, Pikachu/Eevee running across the screen. Mash will not speed it up.
    //Mash A at then end to enter continue screen
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    stream.log("Waiting to enter game...");
    int ret = run_until<JoyconContext>(
        stream, context,
        [&enter_game_timeout](JoyconContext& context){
            pbf_wait(context, enter_game_timeout);
            pbf_press_button(context, BUTTON_A, 400ms, 10ms);
            pbf_wait(context, 5000ms);
        },
        {detector}
    );
    context.wait_for_all_requests();
    if (ret == 0){
        stream.log("At continue screen.");
    }else{
        stream.log("Timed out waiting to enter game and select continue.", COLOR_RED);
        return false;
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    //Continue your adventure.
    BlackScreenOverWatcher detector2(COLOR_YELLOW, {0.2, 0.2, 0.6, 0.6});
    int ret2 = run_until<JoyconContext>(
        stream, context,
        [](JoyconContext& context){
            pbf_press_button(context, BUTTON_A, 400ms, 10ms);
            pbf_wait(context, 5000ms);
        },
        {detector2}
    );
    context.wait_for_all_requests();
    if (ret2 == 0){
        stream.log("Entered game!");
        return true;
    }else{
        stream.log("Timed out waiting to enter game.", COLOR_RED);
        return false;
    }
}

bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, JoyconContext& context,
    Milliseconds post_wait_time
){
    if (dynamic_cast<RightJoycon*>(&context.controller()) == nullptr){
        console.log("Right Joycon required!", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "reset_game_from_home(): Right Joycon required.",
            console
        );
    }
    bool ok = true;
    ok &= reset_game_to_gamemenu(console, context);
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


void reset_game_from_game(
    ProgramEnvironment& env,
    ConsoleHandle& console, JoyconContext& context,
    std::atomic<uint64_t>* errors,
    Milliseconds post_wait_time
){
    while (true){
        pbf_press_button(context, BUTTON_HOME, 200ms, 2000ms);
        if (reset_game_from_home(env, console, context, 3000ms)){
            break;
        }
        env.log("Resetting...", COLOR_RED);
        if (errors){
            (*errors)++;
            env.update_stats();
        }
    }

}



}
}
}
