/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
#include "NintendoSwitch/Inference/NintendoSwitch_CheckOnlineDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_FailedToConnectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_CloseGameDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch_GameEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



//
//  ensure_at_home()
//

void go_home(ConsoleHandle& console, ProControllerContext& context){
    console.log("Going Home...");
    pbf_press_button(context, BUTTON_HOME, 160ms, 0ms);
    if (!console.video().snapshot()){
        pbf_wait(context, 640ms);
        return;
    }
    ensure_at_home(console, context);
}
void go_home(ConsoleHandle& console, JoyconContext& context){
    console.log("Going Home...");
    pbf_press_button(context, BUTTON_HOME, 160ms, 0ms);
    if (!console.video().snapshot()){
        pbf_wait(context, 640ms);
        return;
    }
    ensure_at_home(console, context);
}

template <typename ControllerContext>
void ensure_at_home(ConsoleHandle& console, ControllerContext& context){
    //  Feedback not available. Just assume we're already on Home.
    if (!console.video().snapshot()){
        pbf_wait(context, 640ms);
        return;
    }

    for (size_t attempts = 0; attempts < 10; attempts++){
        HomeMenuWatcher home_menu(console, 100ms);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context, 5000ms,
            {home_menu}
        );
        if (ret == 0){
            console.log("Home detected.");
            //  While we're on the Home screen, we might as well read the
            //  console type as well.
            if (console.state().console_type_confirmed()){
                return;
            }
            ConsoleTypeDetector_Home detector(console);
            detector.detect_only(console.video().snapshot());
            detector.commit_to_cache();
            return;
        }
        console.log("Unable to detect Home. Pressing Home button...", COLOR_RED);
        pbf_press_button(context, BUTTON_HOME, 160ms, 160ms);
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to find Switch Home",
        console
    );
}

void ensure_at_home(ConsoleHandle& console, ProControllerContext& context){
    ensure_at_home<ProControllerContext>(console, context);
}
void ensure_at_home(ConsoleHandle& console, JoyconContext& context){
    ensure_at_home<JoyconContext>(console, context);
}


//
//  close_game_from_home()
//

void close_game_from_home_blind(ConsoleHandle& console, ProControllerContext& context){
    console.log("close_game_from_home_blind");
    ensure_at_home(console, context);

    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen,
    // regardless of whether the game is initially open or closed.

                                                    // if game initially open.  |  if game initially closed
    pbf_mash_button(context, BUTTON_X, 800ms);      // - Close game.            |  - does nothing
    ssf_press_dpad_ptv(context, DPAD_DOWN);         // - Does nothing.          |  - moves selector away from the closed game to avoid opening it.
    ssf_press_dpad_ptv(context, DPAD_DOWN);         // - Does nothing.          |  - Press Down a second time in case we drop one.
    pbf_mash_button(context, BUTTON_A, 400ms);      // - Confirm close game.    |  - opens an app on the home screen (e.g. Online)
    go_home(console, context);                      // - Does nothing.          |  - goes back to home screen.

    // fail-safe against button drops and unexpected error messages.
    pbf_mash_button(context, BUTTON_X, 400ms);
    pbf_mash_button(context, BUTTON_B, 2800ms);
}

void close_game_from_home_blind(ConsoleHandle& console, JoyconContext& context){
    console.log("close_game_from_home_blind");
    ensure_at_home(console, context);
    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.

    // this sequence will close the game from the home screen,
    // regardless of whether the game is initially open or closed.

                                                        // if game initially open.  |  if game initially closed
    pbf_mash_button(context, BUTTON_X, 800ms);          // - Close game.            |  - does nothing
    pbf_move_joystick(context, {0, -1}, 100ms, 10ms);   // - Does nothing.          |  - moves selector away from the closed game to avoid opening it.
    pbf_move_joystick(context, {0, -1}, 100ms, 10ms);   // - Does nothing.          |  - Press Down a second time in case we drop one.
    pbf_mash_button(context, BUTTON_A, 400ms);          // - Confirm close game.    |  - opens an app on the home screen (e.g. Online)
    go_home(console, context);                          // - Does nothing.          |  - goes back to home screen.

    // fail-safe against button drops and unexpected error messages.
    pbf_mash_button(context, BUTTON_X, 400ms);
    pbf_mash_button(context, BUTTON_B, 2000ms);
}

template <typename ControllerContext>
void close_game_from_home(ConsoleHandle& console, ControllerContext& context){
    if (!console.video().snapshot()){  // no visual feedback available
        close_game_from_home_blind(console, context);
        return;
    }

    console.log("close_game_from_home");
    ensure_at_home(console, context);

    // this sequence will close the game from the home screen,
    // regardless of whether the game is initially open or closed.
    bool seen_close_game = false;
    size_t times_seen_home_before = 0;
    while (true){
        context.wait_for_all_requests();

        CloseGameWatcher close_game(console);
        HomeMenuWatcher home(console);
        StartGameUserSelectWatcher user_select(console, COLOR_GREEN);
        int ret = wait_until(
            console, context,
            Seconds(10), 
            {
                close_game,
                home,
                user_select,
            }
        );

        switch(ret){
        case 0: // close_game
            console.log("Detected close game menu.");
            pbf_mash_button(context, BUTTON_A, 100ms);
            seen_close_game = true;
            continue;
        case 1: // home
            if (seen_close_game){ // successfully closed the game
                return;
            }

            if (times_seen_home_before == 0){  // Try closing the game
                pbf_mash_button(context, BUTTON_X, 800ms);
                times_seen_home_before++;
                continue;
            }

            if (times_seen_home_before == 1){  // The game not being selected can happen in Switch 2, when you touch the touchscreen on empty space on the Home screen.
                console.log("Failed to close game once. Either game is already closed, or the game is not selected.");
                ssf_issue_scroll(context, DPAD_DOWN, 24ms);     // moving the DPAD/joystick will allow the selection to come back.
                ssf_issue_scroll(context, DPAD_DOWN, 24ms);
                ssf_issue_scroll(context, DPAD_DOWN, 24ms);
                console.log("Click the Home button to ensure that current game is selected.");
                pbf_press_button(context, BUTTON_HOME, 160ms, 500ms);  // clicking home ensures that the cursor is selected on the current game.
                go_home(console, context);
                console.log("Try again to close the game.");
                pbf_mash_button(context, BUTTON_X, 800ms);   // try again to close the game.
                times_seen_home_before++;
                continue;
            }

            if (times_seen_home_before == 2){
                console.log("Game was already closed.");
                return;
            }

            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "close_game_from_home: Unexpected state.");  

        case 2:
            console.log("Detected user select. (unexpected)", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 500ms);
            seen_close_game = true;
            continue;

        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "close_game_from_home(): Failed to detect either the Home screen or the Close game menu after 10 seconds.",
                console
            );
        }
    }

}

void close_game_from_home(ConsoleHandle& console, ProControllerContext& context){
    close_game_from_home<ProControllerContext>(console, context);
}

void close_game_from_home(ConsoleHandle& console, JoyconContext& context){
    close_game_from_home<JoyconContext>(console, context);
}


//
//  resume_game_from_home()
//

void resume_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool skip_home_press
){
    if (!skip_home_press){
        pbf_press_button(context, BUTTON_HOME, 80ms, 80ms);
    }
    context.wait_for_all_requests();

    while (true){
        {
            UpdateMenuWatcher update_detector(console);
            int ret = wait_until(
                console, context,
                std::chrono::milliseconds(1000),
                { update_detector }
            );
            if (ret == 0){
                console.log("Detected update window.", COLOR_RED);

                pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
                pbf_press_button(context, BUTTON_A, 80ms, 4000ms);
                context.wait_for_all_requests();
                continue;
            }
        }

        //  In case we failed to enter the game.
        HomeMenuWatcher home_detector(console);
        if (home_detector.detect(console.video().snapshot())){
            console.log("Failed to re-enter game. Trying again...", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 80ms, 80ms);
            continue;
        }else{
            break;
        }
    }
}
void resume_game_from_home(
    ConsoleHandle& console, JoyconContext& context,
    bool skip_home_press
){
    if (!skip_home_press){
        pbf_press_button(context, BUTTON_HOME, 20ms, 10ms);
    }
    context.wait_for_all_requests();

    while (true){
        {
            UpdateMenuWatcher update_detector(console);
            int ret = wait_until(
                console, context,
                std::chrono::milliseconds(1000),
                { update_detector }
            );
            if (ret == 0){
                console.log("Detected update window.", COLOR_RED);

                pbf_move_joystick(context, {0, +1}, 10ms, 0ms);
                pbf_press_button(context, BUTTON_A, 10ms, 500ms);
                context.wait_for_all_requests();
                continue;
            }
        }

        //  In case we failed to enter the game.
        HomeMenuWatcher home_detector(console);
        if (home_detector.detect(console.video().snapshot())){
            console.log("Failed to re-enter game. Trying again...", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 20ms, 10ms);
            continue;
        }else{
            break;
        }
    }
}



//
//  move_to_user()
//

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
void move_to_user(JoyconContext& context, uint8_t user_slot){
    if (user_slot != 0){
        //  Move to correct user.
        for (uint8_t c = 0; c < 9; c++){    //  Extra iteration in case one gets dropped.
            pbf_move_joystick(context, {-1, 0}, 160ms, 160ms);
        }
        for (uint8_t c = 1; c < user_slot; c++){
            pbf_move_joystick(context, {+1, 0}, 160ms, 160ms);
        }
    }
}



//
//  start_game_from_home_blind()
//

void start_game_from_home_blind(
    Logger& logger, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
){
    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            ssf_press_dpad_ptv(context, DPAD_RIGHT, 80ms);
        }
    }

    if (tolerate_update_menu){
        if (ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET){
            throw UserSetupError(
                logger,
                "Cannot have both \"Tolerate Update Menu\" and \"Start Game Requires Internet\" enabled at the same time without video feedback."
            );
        }

        //  If the update menu isn't there, these will get swallowed by the opening
        //  animation for the select user menu.
        pbf_press_button(context, BUTTON_A, 80ms, 1400ms);   //  Choose game
        pbf_press_dpad(context, DPAD_UP, 80ms, 0ms);    //  Skip the update window.
        move_to_user(context, user_slot);
    }

//    cout << "START_GAME_REQUIRES_INTERNET = " << ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET << endl;
    if (!ConsoleSettings::instance().START_GAME_REQUIRES_INTERNET && user_slot == 0){
        //  Mash your way into the game.
        pbf_mash_button(context, BUTTON_A, start_game_mash);
    }else{
        pbf_press_button(context, BUTTON_A, 80ms, 1400ms);   //  Enter select user menu.
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



//
//  start_game_from_home_with_inference()
//

void start_game_from_home_with_inference(
    ConsoleHandle& console, ProControllerContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_wait
){
    context.wait_for_all_requests();
    {
        HomeMenuWatcher detector(console);
        int ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10000ms);
            },
            { detector }
        );
        if (ret == 0){
            console.log("Detected Home screen.");
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): Failed to detect Home screen after 10 seconds.",
                console
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

    pbf_press_button(context, BUTTON_A, 160ms, 840ms);

    WallClock deadline = current_time() + std::chrono::minutes(5);
    while (current_time() < deadline){
        HomeMenuWatcher home(console, std::chrono::milliseconds(2000));
        StartGameUserSelectWatcher user_select(console, COLOR_GREEN);
        UpdateMenuWatcher update_menu(console, COLOR_PURPLE);
        CheckOnlineWatcher check_online(COLOR_CYAN);
        FailedToConnectWatcher failed_to_connect(COLOR_YELLOW);
        BlackScreenWatcher black_screen(COLOR_BLUE, {0.1, 0.15, 0.8, 0.7});
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                home,
                user_select,
                update_menu,
                check_online,
                failed_to_connect,
                black_screen,
            }
        );

        //  Wait for screen to stabilize.
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected home screen (again).", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 1:
            console.log("Detected user-select screen.");
            move_to_user(context, user_slot);
            pbf_press_button(context, BUTTON_A, 80ms, start_game_wait);
            break;
        case 2:
            console.log("Detected update menu.", COLOR_BLUE);
            pbf_press_dpad(context, DPAD_UP, 40ms, 0ms);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 3:
            console.log("Detected check online.", COLOR_BLUE);
            context.wait_for(std::chrono::seconds(1));
            break;
        case 4:
            console.log("Detected failed to connect.", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 5:
            console.log("Detected black screen. Game started...");
            return;
        default:
            console.log("start_game_from_home_with_inference(): No recognizable state after 30 seconds.", COLOR_RED);
            pbf_press_button(context, BUTTON_HOME, 160ms, 840ms);
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "start_game_from_home_with_inference(): Failed to start game after 5 minutes.",
        console
    );
}
void start_game_from_home_with_inference(
    ConsoleHandle& console, JoyconContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_wait
){
    context.wait_for_all_requests();

    if (dynamic_cast<RightJoycon*>(&context.controller()) == nullptr){
        console.log("Right Joycon required!", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "start_game_from_home_with_inference(): Right Joycon required.",
            console
        );
    }

    {
        HomeMenuWatcher detector(console);
        int ret = run_until<JoyconContext>(
            console, context,
            [](JoyconContext& context){
                pbf_mash_button(context, BUTTON_B, 10000ms);
            },
            { detector }
        );
        if (ret == 0){
            console.log("Detected Home screen.");
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): Failed to detect Home screen after 10 seconds.",
                console
            );
        }
        context.wait_for(std::chrono::milliseconds(100));
    }

    if (game_slot != 0){
        ssf_press_button(context, BUTTON_HOME, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0, 160ms);
        for (uint8_t c = 1; c < game_slot; c++){
            pbf_move_joystick(context, {+1, 0}, 160ms, 0ms);
        }
        context.wait_for_all_requests();
    }

    pbf_press_button(context, BUTTON_A, 160ms, 840ms);

    while (true){
        HomeMenuWatcher home(console, std::chrono::milliseconds(2000));
        StartGameUserSelectWatcher user_select(console, COLOR_GREEN);
        UpdateMenuWatcher update_menu(console, COLOR_PURPLE);
        CheckOnlineWatcher check_online(COLOR_CYAN);
        FailedToConnectWatcher failed_to_connect(COLOR_YELLOW);
        BlackScreenWatcher black_screen(COLOR_BLUE);
        context.wait_for_all_requests();
        int ret = wait_until(
            console, context,
            std::chrono::seconds(30),
            {
                home,
                user_select,
                update_menu,
                check_online,
                failed_to_connect,
                black_screen,
            }
        );

        //  Wait for screen to stabilize.
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            console.log("Detected home screen (again).", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 1:
            console.log("Detected user-select screen.");
            move_to_user(context, user_slot);
            pbf_press_button(context, BUTTON_A, 80ms, start_game_wait);
            break;
        case 2:
            console.log("Detected update menu.", COLOR_BLUE);
            pbf_move_joystick(context, {0, +1}, 50ms, 0ms);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 3:
            console.log("Detected check online.", COLOR_BLUE);
            context.wait_for(std::chrono::seconds(1));
            break;
        case 4:
            console.log("Detected failed to connect.", COLOR_BLUE);
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        case 5:
            console.log("Detected black screen. Game started...");
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "start_game_from_home_with_inference(): No recognizable state after 30 seconds.",
                console
            );
        }
    }
}



//
//  start_game_from_home()
//

void start_game_from_home(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_mash
){
    context.wait_for_all_requests();
    if (console.video().snapshot()){
        console.log("start_game_from_home(): Video capture available. Using inference...");
        start_game_from_home_with_inference(
            console, context,
            game_slot, user_slot,
            start_game_mash
        );
    }else{
        console.log("start_game_from_home(): Video capture not available.", COLOR_RED);
        start_game_from_home_blind(
            console, context,
            tolerate_update_menu,
            game_slot, user_slot,
            start_game_mash
        );
    }
}
void start_game_from_home(
    ConsoleHandle& console, JoyconContext& context,
    uint8_t game_slot,
    uint8_t user_slot,
    Milliseconds start_game_wait
){
    //  Inference is required.
    start_game_from_home_with_inference(
        console, context,
        game_slot, user_slot,
        start_game_wait
    );
}



//
//  from_home_close_and_reopen_game()
//

void from_home_close_and_reopen_game(
    ConsoleHandle& console, ProControllerContext& context,
    bool tolerate_update_menu
){
    Milliseconds start_game_mash = ConsoleSettings::instance().START_GAME_MASH0;

#if 1
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
            start_game_mash
        );
        return;
    }
#endif

    //  Fastest setting. No internet needed and no update menu.
    ssf_mash1_button(context, BUTTON_X, 400ms);

    //  Use mashing to ensure that the X press succeeds. If it fails, the SR
    //  will fail and can kill a den for the autohosts.
    ssf_mash2_button(context, BUTTON_X, BUTTON_A, 3000ms + start_game_mash);
    ssf_mash1_button(context, BUTTON_X, start_game_mash);
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



//
//  openedgame_to_gamemenu()
//

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
bool openedgame_to_gamemenu(
    VideoStream& stream, JoyconContext& context,
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
