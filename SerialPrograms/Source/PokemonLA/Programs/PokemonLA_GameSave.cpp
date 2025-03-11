/*  Game Save
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA_GameSave.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace{

const ImageFloatBox tab_box{0.450, 0.005, 0.040, 0.010};
const ImageFloatBox save_icon_box{0.456, 0.015, 0.026, 0.041};

}


bool save_tab_selected(const ImageViewRGB32 &screen){

    const ImageStats stats = image_stats(extract_box_reference(screen, tab_box));
    return (stats.stddev.sum() < 15 &&
        stats.average.b > stats.average.r && stats.average.b > stats.average.g
    );
}

bool save_tab_disabled(const ImageViewRGB32 &screen){

    // Replace white background with zero-alpha color so that they won't be counted in
    // the following image_stats()
    // The white background is defined as the color between 0xffa0a0a0 and 0xffffffff.
    const bool replace_background = true;
    ImageRGB32 region = filter_rgb32_range(
        extract_box_reference(screen, save_icon_box),
        0xffa0a0a0, 0xffffffff, Color(0), replace_background
    );

    ImageStats stats = image_stats(region);
    // cout << "color " << stats.count << " " << stats.average.to_string() << endl;
    return (stats.average.r > stats.average.b + 50.0);
}


bool save_game_from_overworld(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context
){
    OverlayBoxScope tab_box_scope(stream.overlay(), tab_box);
    OverlayBoxScope icon_box_scope(stream.overlay(), save_icon_box);
    stream.log("Saving game...");
    stream.overlay().add_log("Saving game...", COLOR_WHITE);

    // Press DPAD_UP to open menu
    pbf_press_dpad(context, DPAD_UP, 20, 120);
    context.wait_for_all_requests();
    auto snapshot = stream.video().snapshot();
    if (save_tab_disabled(snapshot)){
        return false;
    }

    bool found = false;
    for (size_t c = 0; c < 10; c++){
        if (save_tab_selected(snapshot)){
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_press_button(context, BUTTON_B, 20, 105);
//            pbf_press_button(context, BUTTON_B, 20, 105);
            context.wait_for_all_requests();
            found = true;
            break;
        }
        pbf_press_button(context, BUTTON_ZR, 20, 80);
        context.wait_for_all_requests();
        snapshot = stream.video().snapshot();
    }
    if (!found){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to find save menu.",
            stream
        );
    }

    ArcPhoneDetector detector(stream.logger(), stream.overlay(), std::chrono::milliseconds(100), true);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [&](ProControllerContext& context){
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_B, 20, 230);
            }
        },
        {detector}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to return to overworld.",
            stream
        );
    }
    stream.log("Saving game... Done.");

    return true;
}





}
}
}
