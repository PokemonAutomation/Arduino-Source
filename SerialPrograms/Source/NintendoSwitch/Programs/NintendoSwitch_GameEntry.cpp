/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "NintendoSwitch_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



void resume_game_from_home(
    VideoStream& stream, ProControllerContext& context,
    bool skip_home_press
){
    if (!skip_home_press){
        pbf_press_button(context, BUTTON_HOME, 10, 10);
    }
    context.wait_for_all_requests();

    while (true){
        {
            UpdateMenuWatcher update_detector;
            int ret = wait_until(
                stream, context,
                std::chrono::milliseconds(1000),
                { update_detector }
            );
            if (ret == 0){
                stream.log("Detected update window.", COLOR_RED);

                pbf_press_dpad(context, DPAD_UP, 5, 0);
                pbf_press_button(context, BUTTON_A, 10, 500);
                context.wait_for_all_requests();
                continue;
            }
        }

        //  In case we failed to enter the game.
        HomeWatcher home_detector;
        if (home_detector.detect(stream.video().snapshot())){
            stream.log("Failed to re-enter game. Trying again...", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 10, 10);
            continue;
        }else{
            break;
        }
    }
}











void move_to_user(ProControllerContext& context, uint8_t user_slot){
    if (user_slot != 0){
        //  Move to correct user.
        for (uint8_t c = 0; c < 9; c++){    //  Extra iteration in case one gets dropped.
            ssf_issue_scroll_ptv(context, DPAD_LEFT, 160ms, 160ms);
        }
        for (uint8_t c = 1; c < user_slot; c++){
            ssf_issue_scroll_ptv(context, DPAD_RIGHT, 160ms, 160ms);
        }
    }
}


void start_game_from_home_with_inference(
    VideoStream& stream, ProControllerContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_wait
){
    context.wait_for_all_requests();
    {
        HomeWatcher detector;
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            },
            { detector }
        );
        if (ret == 0){
            stream.log("Detected Home screen.");
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): Failed to detect Home screen after 10 seconds.",
                stream
            );
        }
        context.wait_for(std::chrono::milliseconds(100));
    }

    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            ssf_press_dpad_ptv(context, DPAD_RIGHT, 160ms);
        }
        context.wait_for_all_requests();
    }

    pbf_press_button(context, BUTTON_A, 20, 105);

    while (true){
        HomeWatcher home(std::chrono::milliseconds(2000));
        StartGameUserSelectWatcher user_select(COLOR_GREEN);
        UpdateMenuWatcher update_menu(COLOR_PURPLE);
        CheckOnlineWatcher check_online(COLOR_CYAN);
        BlackScreenWatcher black_screen(COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {
                home,
                user_select,
                update_menu,
                check_online,
                black_screen,
            }
        );

        //  Wait for screen to stabilize.
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            stream.log("Detected home screen (again).", COLOR_RED);
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 1:
            stream.log("Detected user-select screen.");
            move_to_user(context, user_slot);
            pbf_press_button(context, BUTTON_A, 80ms, start_game_wait);
            break;
        case 2:
            stream.log("Detected update menu.", COLOR_RED);
            pbf_press_dpad(context, DPAD_UP, 5, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            break;
        case 3:
            stream.log("Detected check online.", COLOR_RED);
            context.wait_for(std::chrono::seconds(1));
            break;
        case 4:
            stream.log("Detected black screen. Game started...");
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): No recognizable state after 30 seconds.",
                stream
            );
        }
    }
}


void start_game_from_home(
    VideoStream& stream, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
){
    context.wait_for_all_requests();
    if (stream.video().snapshot()){
        stream.log("start_game_from_home(): Video capture available. Using inference...");
        start_game_from_home_with_inference(stream, context, game_slot, user_slot, start_game_mash);
        return;
    }else{
        stream.log("start_game_from_home(): Video capture not available.", COLOR_RED);
    }

    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            ssf_press_dpad_ptv(context, DPAD_RIGHT, 80ms);
        }
    }

    if (tolerate_update_menu){
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            throw UserSetupError(
                stream.logger(),
                "Cannot have both \"Tolerate Update Menu\" and \"Start Game Requires Internet\" enabled at the same time without video feedback."
            );
        }

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(context, BUTTON_A, 10, 175);    //  Choose game
        pbf_press_dpad(context, DPAD_UP, 10, 0);         //  Skip the update window.
        move_to_user(context, user_slot);
    }

//    cout << "START_GAME_REQUIRES_INTERNET = " << ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET << endl;
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(context, BUTTON_A, start_game_mash);
    }else{
        pbf_press_button(context, BUTTON_A, 10, 175);   //  Enter select user menu.
        move_to_user(context, user_slot);
        ssf_press_button_ptv(context, BUTTON_A, 160ms); //  Enter game

        //  Switch to mashing ZL instead of A to get into the game.
        //  Mash your way into the game.
        Milliseconds duration = start_game_mash;
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            //  Need to wait a bit longer for the internet check.
            duration += ConsoleSettings::instance().START_GAME_INTERNET_CHECK_DELAY0;
        }
//        pbf_mash_button(context, BUTTON_ZL, duration);
        pbf_wait(context, duration);
    }
    context.wait_for_all_requests();
}



class GameLoadingDetector : public VisualInferenceCallback{
public:
    GameLoadingDetector(bool invert)
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
    VideoStream& stream, ProControllerContext& context,
    Milliseconds timeout
){
    {
        stream.log("Waiting to load game...");
        GameLoadingDetector detector(false);
        int ret = wait_until(
            stream, context,
            timeout,
            {{detector}}
        );
        if (ret < 0){
            stream.log("Timed out waiting to enter game.", COLOR_RED);
            return false;
        }
    }
    {
        stream.log("Waiting for game menu...");
        GameLoadingDetector detector(true);
        int ret = wait_until(
            stream, context,
            timeout,
            {{detector}}
        );
        if (ret < 0){
            stream.log("Timed out waiting for game menu.", COLOR_RED);
            return false;
        }
    }
    return true;
}









}
}
