/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "NintendoSwitch_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


void move_to_user(BotBaseContext& context, uint8_t user_slot){
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


void open_game_from_home_with_inference(
    ConsoleHandle& console,
    BotBaseContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_wait
){
    context.wait_for_all_requests();
    {
        HomeWatcher detector;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            },
            { detector }
        );
        if (ret == 0){
            console.log("Detected Home screen.");
        }else{
            console.log("Failed to detect Home screen after 10 seconds.", COLOR_RED);
        }
        context.wait_for(std::chrono::milliseconds(100));
    }

    if (game_slot != 0){
        pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 5, 5);
        }
        context.wait_for_all_requests();
    }

    while (true){
        StartGameUserSelectWatcher detector;
        int ret = run_until(
            console, context,
            [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_A, 20, 3 * TICKS_PER_SECOND);
            },
            { detector }
        );
        if (ret == 0){
            console.log("Detected user-select screen.");
            break;
        }
        console.log("Failed to detect user-select screen after 3 seconds. Attempting to dodge possible update window.", COLOR_RED);

        //  Skip the update window and try again.
        pbf_press_dpad(context, DPAD_UP, 5, 0);
        context.wait_for_all_requests();
    }

    //  Wait for screen to stabilize.
    context.wait_for(std::chrono::milliseconds(100));

    //  Move to user and enter game.
    move_to_user(context, user_slot);
    pbf_press_button(context, BUTTON_A, 10, start_game_wait);
}


void open_game_from_home(
    ConsoleHandle& console,
    BotBaseContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    uint16_t start_game_mash
){
    if (console.video().snapshot()){
        console.log("open_game_from_home(): Video capture available. Using inference...");
        open_game_from_home_with_inference(console, context, game_slot, user_slot, start_game_mash);
        return;
    }else{
        console.log("open_game_from_home(): Video capture not available.", COLOR_RED);
    }

    if (game_slot != 0){
        pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY - 10);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 5, 5);
        }
    }

    if (tolerate_update_menu){
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            throw UserSetupError(
                console.logger(),
                "Cannot have both \"Tolerate Update Menu\" and \"Start Game Requires Internet\" enabled at the same time without video feedback."
            );
        }

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(context, BUTTON_A, 5, 175);    //  Choose game
        pbf_press_dpad(context, DPAD_UP, 5, 0);         //  Skip the update window.
        move_to_user(context, user_slot);
    }

//    cout << "START_GAME_REQUIRES_INTERNET = " << ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET << endl;
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(context, BUTTON_A, start_game_mash);
    }else{
        pbf_press_button(context, BUTTON_A, 5, 175);    //  Enter select user menu.
        move_to_user(context, user_slot);
        pbf_press_button(context, BUTTON_A, 5, 5);      //  Enter game

        //  Switch to mashing ZL instead of A to get into the game.
        //  Mash your way into the game.
        uint16_t duration = start_game_mash;
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY;
        }
//        pbf_mash_button(context, BUTTON_ZL, duration);
        pbf_wait(context, duration);
    }
    context.wait_for_all_requests();
}



class LoadingDetector : public VisualInferenceCallback{
public:
    LoadingDetector(bool invert)
        : VisualInferenceCallback("LoadingDetector")
        , m_box0(0.2, 0.2, 0.6, 0.1)
        , m_box1(0.2, 0.7, 0.6, 0.1)
        , m_invert(invert)
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(COLOR_RED, m_box0);
        items.add(COLOR_RED, m_box1);
    }

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        if (!is_black(extract_box_reference(frame, m_box0))){
            return m_invert;
        }
        if (!is_black(extract_box_reference(frame, m_box1))){
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
    ConsoleHandle& console, BotBaseContext& context,
    uint16_t timeout
){
    {
        console.log("Waiting to load game...");
        LoadingDetector detector(false);
        int ret = wait_until(
            console, context,
            std::chrono::milliseconds(timeout * (1000 / TICKS_PER_SECOND)),
            {{detector}}
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
            console, context,
            std::chrono::milliseconds(timeout * (1000 / TICKS_PER_SECOND)),
            {{detector}}
        );
        if (ret < 0){
            console.log("Timed out waiting for game menu.", COLOR_RED);
            return false;
        }
    }
    return true;
}









}
}
