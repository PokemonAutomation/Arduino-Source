/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Routines.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_GameEntry.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class LoadingDetector : public VisualInferenceCallback{
public:
    LoadingDetector(bool invert)
        : m_box0(0.2, 0.2, 0.6, 0.1)
        , m_box1(0.2, 0.7, 0.6, 0.1)
        , m_invert(invert)
    {}

    virtual void make_overlays(OverlaySet& items) const override{
        items.add(COLOR_RED, m_box0);
        items.add(COLOR_RED, m_box1);
    }

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override{
        if (!is_black(extract_box(frame, m_box0))){
            return m_invert;
        }
        if (!is_black(extract_box(frame, m_box1))){
            return m_invert;
        }
        return !m_invert;
   }

private:
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    bool m_invert;
};



bool openedgame_to_gamemenu(
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t timeout
){
    {
        console.log("Waiting to load game...");
        LoadingDetector detector(false);
        int ret = wait_until(
            env, console,
            std::chrono::milliseconds(timeout * (1000 / TICKS_PER_SECOND)),
            { &detector }
        );
        if (ret < 0){
            console.log("Timed out waiting to enter game.", COLOR_RED);
            return false;
        }
    }
    {
        console.log("Waiting for game menu...");
        LoadingDetector detector(true);
        int ret = wait_until(
            env, console,
            std::chrono::milliseconds(timeout * (1000 / TICKS_PER_SECOND)),
            { &detector }
        );
        if (ret < 0){
            console.log("Timed out waiting for game menu.", COLOR_RED);
            return false;
        }
    }
    return true;
}
bool gamemenu_to_ingame(
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t mash_duration, uint16_t enter_game_timeout
){
    console.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector({0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(console, BUTTON_ZL, mash_duration);
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
bool openedgame_to_ingame(
    ProgramEnvironment& env, ConsoleHandle& console,
    uint16_t load_game_timeout,
    uint16_t mash_duration, uint16_t enter_game_timeout,
    uint16_t post_wait_time
){
    bool ok = true;
    ok &= openedgame_to_gamemenu(env, console, load_game_timeout);
    ok &= gamemenu_to_ingame(env, console, mash_duration, enter_game_timeout);
    if (!ok){
        dump_image(env.logger(), env.program_info(), "StartGame", console.video().snapshot());
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(console, post_wait_time);
    console.botbase().wait_for_all_requests();
    return ok;
}

void move_to_user(const BotBaseContext& context, uint8_t user_slot){
    if (user_slot != 0){
        //  Move to correct user.
        for (uint8_t c = 0; c < 8; c++){
            pbf_press_dpad(context, DPAD_LEFT, 7, 7);
        }
        for (uint8_t c = 1; c < user_slot; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 7, 7);
        }
    }
}

void start_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_mash
){
    if (game_slot != 0){
        pbf_press_button(console, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(console, DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(console, BUTTON_A, 5, 175);      //  Choose game
        pbf_press_dpad(console, DPAD_UP, 5, 0);          //  Skip the update window.
        move_to_user(console, user_slot);
    }

//    cout << "START_GAME_REQUIRES_INTERNET = " << START_GAME_REQUIRES_INTERNET << endl;
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(console, BUTTON_A, start_game_mash);
    }else{
        pbf_press_button(console, BUTTON_A, 5, 175);     //  Enter select user menu.
        move_to_user(console, user_slot);
        pbf_press_button(console, BUTTON_A, 5, 5);       //  Enter game

        //  Switch to mashing ZL instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = start_game_mash;
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY;
        }
        pbf_mash_button(console, BUTTON_ZL, duration);
    }
    console.botbase().wait_for_all_requests();
}



bool reset_game_from_home(
    ProgramEnvironment& env, ConsoleHandle& console,
    bool tolerate_update_menu,
    uint16_t post_wait_time
){
    if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET || tolerate_update_menu){
        close_game(console);
        start_game_from_home(
            env, console,
            tolerate_update_menu,
            0, 0,
            GameSettings::instance().START_GAME_MASH
        );
    }else{
        pbf_press_button(console, BUTTON_X, 50, 0);
        pbf_mash_button(console, BUTTON_A, GameSettings::instance().START_GAME_MASH);
    }
    bool ret = openedgame_to_ingame(
        env, console,
        GameSettings::instance().START_GAME_WAIT,
        GameSettings::instance().ENTER_GAME_MASH,
        GameSettings::instance().ENTER_GAME_WAIT,
        post_wait_time
    );
    console.botbase().wait_for_all_requests();
    return ret;
}



}
}
}
