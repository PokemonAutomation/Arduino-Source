/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
//#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_GameEntry.h"

#include <iostream>
using std::cout;
using std::endl;

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


bool reset_game_to_gamemenu(ConsoleHandle& console, BotBaseContext& context){
    close_game(context);
    start_game_from_home(
        console,
        context,
        true,
        0, 0,
        GameSettings::instance().START_GAME_MASH
    );

    uint16_t timeout = GameSettings::instance().START_GAME_WAIT;

    {
        console.log("Waiting to load game...");
        WaitforWhiteLoadScreen detector(false);
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
        WaitforWhiteLoadScreen detector(true);
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

//    // Now the game has opened:
//    return openedgame_to_gamemenu(console, context, GameSettings::instance().START_GAME_WAIT);
    return true;
}

bool gamemenu_to_ingame(ConsoleHandle& console, BotBaseContext& context){
    console.log("Mashing A to enter game...");
    BlackScreenOverWatcher detector(COLOR_RED, {0.2, 0.2, 0.6, 0.6});
    pbf_mash_button(context, BUTTON_A, GameSettings::instance().ENTER_GAME_MASH);
    context.wait_for_all_requests();
    console.log("Waiting to enter game...");
    int ret = wait_until(
        console, context,
        std::chrono::milliseconds(GameSettings::instance().ENTER_GAME_WAIT * (1000 / TICKS_PER_SECOND)),
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

bool reset_game_from_home(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint16_t post_wait_time
){
    console.log("Resetting game from home...");
    console.overlay().add_log("Reset game", COLOR_WHITE);
    bool ok = true;
    ok &= reset_game_to_gamemenu(console, context);
    ok &= gamemenu_to_ingame(console, context);
    if (!ok){
        dump_image(console.logger(), info, "StartGame", console.video().snapshot());
    }
    console.log("Entered game! Waiting out grace period.");
    pbf_wait(context, post_wait_time);
    context.wait_for_all_requests();
    return ok;
}








}
}
}
