/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_GameEntry.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


bool gamemenu_to_ingame(
    ConsoleHandle& console, BotBaseContext& context,
    uint16_t mash_duration, uint16_t enter_game_timeout
){
    console.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(context, BUTTON_A, mash_duration);
    context.wait_for_all_requests();
    console.log("Waiting to enter game...");
    int ret = wait_until(
        console, context,
        std::chrono::milliseconds(enter_game_timeout * (1000 / TICKS_PER_SECOND)),
        {{detector}}
    );
    if (ret == 0){
        console.log("Entered game!");
        return true;
    }else{
        console.log("Timed out waiting to enter game.", COLOR_RED);
        return false;
    }
}

bool switch_home_to_gamemenu(
    ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu
){
    if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game(context);
        start_game_from_home(
            console,
            context,
            tolerate_update_menu,
            0, 0,
            GameSettings::instance().START_GAME_MASH
        );
    }else{
        pbf_press_button(context, BUTTON_X, 50, 0);
        pbf_mash_button(context, BUTTON_A, GameSettings::instance().START_GAME_MASH);
    }

    // Now the game has opened:
    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT0);
}

bool reset_game_from_home(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    bool ok = true;
    ok &= switch_home_to_gamemenu(console, context, tolerate_update_menu);
    ok &= gamemenu_to_ingame(console, context, GameSettings::instance().ENTER_GAME_MASH, GameSettings::instance().ENTER_GAME_WAIT);
    if (!ok){
        dump_image(console.logger(), env.program_info(), "StartGame", console.video().snapshot());
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}





}
}
}
