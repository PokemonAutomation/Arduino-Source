/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_GameEntry.h"


//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class WaitforWhiteLoadScreen : public VisualInferenceCallback{
public:
    WaitforWhiteLoadScreen(bool invert)
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
        if (!is_white(extract_box_reference(frame, m_box0))){
            return m_invert;
        }
        if (!is_white(extract_box_reference(frame, m_box1))){
            return m_invert;
        }
        return !m_invert;
   }

private:
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    bool m_invert;
};


bool reset_game_to_gamemenu(ConsoleHandle& console, ProControllerContext& context){
    close_game_from_home(console, context);
    start_game_from_home(
        console, context,
        true,
        0, 0,
        GameSettings::instance().START_GAME_MASH0
    );

    Milliseconds timeout = GameSettings::instance().START_GAME_WAIT0;

    {
        console.log("Waiting to load game...");
        WaitforWhiteLoadScreen detector(false);
        int ret = wait_until(
            console, context,
            timeout,
            {{detector}}
        );
        if (ret < 0){
            console.log("Timed out waiting to enter game.", COLOR_RED);
            return false;
        }
    }
    {
        console.log("Waiting for game menu...");
        WaitforWhiteLoadScreen detector(true);
        int ret = wait_until(
            console, context,
            timeout,
            {{detector}}
        );
        if (ret < 0){
            console.log("Timed out waiting for game menu.", COLOR_RED);
            return false;
        }
    }

//    // Now the game has opened:
//    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT);
    return true;
}

bool gamemenu_to_ingame(VideoStream& stream, ProControllerContext& context){
    stream.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(context, BUTTON_A, GameSettings::instance().ENTER_GAME_MASH0);
    context.wait_for_all_requests();
    stream.log("Waiting to enter game...");
    int ret = wait_until(
        stream, context,
        GameSettings::instance().ENTER_GAME_WAIT0,
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
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    uint16_t post_wait_time
){
    console.log("Resetting game from home...");
    console.overlay().add_log("Reset game", COLOR_WHITE);
    bool ok = true;
    ok &= reset_game_to_gamemenu(console, context);
    ok &= gamemenu_to_ingame(console, context);
    if (!ok){
        dump_image(console.logger(), info, console.video(), "StartGame");
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}
bool reset_game_from_home_zoom_out(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context,
    uint16_t post_wait_time
){
    bool ret = reset_game_from_home(info, console, context, post_wait_time);

    //  5 zooms will guarantee that are fully zoomed out regardless of whether
    //  we are on the DLC update.
    pbf_press_button(context, BUTTON_RCLICK, 20, 105);
    pbf_press_button(context, BUTTON_RCLICK, 20, 105);
    pbf_press_button(context, BUTTON_RCLICK, 20, 105);
    pbf_press_button(context, BUTTON_RCLICK, 20, 105);
    pbf_press_button(context, BUTTON_RCLICK, 20, 105);

    return ret;
}

void reset_game(
    const ProgramInfo& info,
    ConsoleHandle& console, ProControllerContext& context
){
    try{
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        context.wait_for_all_requests();
        if (!reset_game_from_home(info, console, context, 5 * TICKS_PER_SECOND)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to start game.",
                console
            );
        }
    }catch (OperationFailedException& e){
        // To be safe: avoid doing anything outside of game on Switch,
        // make game resetting non error recoverable
        throw FatalProgramException(std::move(e));
    }
}






}
}
}
