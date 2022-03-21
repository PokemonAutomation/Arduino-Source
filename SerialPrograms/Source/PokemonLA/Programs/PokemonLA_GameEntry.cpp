/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
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
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t mash_duration, uint16_t enter_game_timeout
){
    console.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(console, BUTTON_A, mash_duration);
    console.botbase().wait_for_all_requests();
    console.log("Waiting to enter game...");
    int ret = wait_until(
        env, console,
        std::chrono::milliseconds(enter_game_timeout * (1000 / TICKS_PER_SECOND)),
        { &detector }
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
    ProgramEnvironment& env, ConsoleHandle& console,
    bool tolerate_update_menu
){
    if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game(console);
        open_game_from_home(
            env, console,
            tolerate_update_menu,
            0, 0,
            GameSettings::instance().START_GAME_MASH
        );
    }else{
        pbf_press_button(console, BUTTON_X, 50, 0);
        pbf_mash_button(console, BUTTON_A, GameSettings::instance().START_GAME_MASH);
    }

    // Now the game has opened:
    return openedgame_to_gamemenu(env, console, GameSettings::instance().START_GAME_WAIT0);
}

bool reset_game_from_home(
    ProgramEnvironment& env, ConsoleHandle& console,
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    bool ok = true;
    ok &= switch_home_to_gamemenu(env, console, tolerate_update_menu);
    ok &= gamemenu_to_ingame(env, console, GameSettings::instance().ENTER_GAME_MASH, GameSettings::instance().ENTER_GAME_WAIT);
    if (!ok){
        dump_image(env.logger(), env.program_info(), "StartGame", console.video().snapshot());
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(console, post_wait_time);
    console.botbase().wait_for_all_requests();
    return ok;
}



void save_game_from_overworld(ProgramEnvironment& env, ConsoleHandle& console){
    InferenceBoxScope box(console, {0.450, 0.005, 0.040, 0.010});

    console.log("Saving game...");
    pbf_press_dpad(console, DPAD_UP, 20, 355);

    for (size_t c = 0; c < 10; c++){
        console.botbase().wait_for_all_requests();

        ImageStats stats = image_stats(extract_box(console.video().snapshot(), box));
//        is_solid(stats, {0.208333, 0.338542, 0.453125}, 0.15, 15)
        if (stats.stddev.sum() < 15 &&
            stats.average.b > stats.average.r && stats.average.b > stats.average.g
        ){
            pbf_press_button(console, BUTTON_A, 20, 605);
            pbf_press_button(console, BUTTON_B, 20, 230);
            pbf_press_button(console, BUTTON_B, 20, 355);
            return;
        }
        pbf_press_button(console, BUTTON_ZR, 20, 105);
    }

    console.log("Unable to find save menu.", COLOR_RED);
    throw OperationFailedException(console, "Unable to find save menu.");
}





}
}
}
