/*  Max Lair Enter Lobby
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "VideoGames/PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "VideoGames/PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "VideoGames/PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "VideoGames/PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "VideoGames/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh_MaxLair_Run_EnterLobby.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



class GreyDialogDetector : public StaticScreenDetector{
public:
    GreyDialogDetector(Color color = COLOR_RED)
        : m_color(color)
        , m_box0(0.180, 0.815, 0.015, 0.030)
        , m_box1(0.785, 0.840, 0.030, 0.050)
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(m_color, m_box0);
        items.add(m_color, m_box1);
    }
    virtual bool detect(const ImageViewRGB32& screen) override{
        ImageStats stats0 = image_stats(extract_box_reference(screen, m_box0));
        if (!is_grey(stats0, 0, 200, 10)){
            return false;
        }
        ImageStats stats1 = image_stats(extract_box_reference(screen, m_box1));
        if (!is_grey(stats1, 0, 200, 10)){
            return false;
        }
        return true;
    }

private:
    Color m_color;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
};
class GreyDialogWatcher : public DetectorToFinder<GreyDialogDetector>{
public:
    GreyDialogWatcher(Color color = COLOR_RED)
         : DetectorToFinder("GreyDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};




std::shared_ptr<const ImageRGB32> enter_lobby(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t boss_slot, bool connect_to_internet,
    ReadableQuantity999& ore
){
    pbf_mash_button(context, BUTTON_B, 2000ms);

    if (connect_to_internet){
        connect_to_internet_with_inference(info, stream, context);
    }

//    VideoOverlaySet boxes(stream.overlay());

//    arrow_detector.make_overlays(boxes);
//    dialog_detector.make_overlays(boxes);

//    OverlayBoxScope ore_box(stream.overlay(), {0.900, 0.015, 0.020, 0.040});
    OverlayBoxScope ore_box(stream.overlay(), {0.930, 0.050, 0.065, 0.010});
    OverlayBoxScope ore_quantity(stream.overlay(), {0.945, 0.010, 0.0525, 0.050});

    size_t arrow_count = 0;
    size_t ore_dialog_count = 0;
    VideoSnapshot entrance;
    WallClock deadline = current_time() + std::chrono::seconds(120);
    while (current_time() < deadline){
        YCommIconWatcher overworld(COLOR_CYAN);
        WhiteDialogBoxWatcher white_dialog(COLOR_RED);
        SelectionArrowWatcher arrow_detector(
            stream.overlay(),
            ImageFloatBox(0.350, 0.450, 0.500, 0.400)
        );
        GreyDialogWatcher big_dialog(COLOR_RED);
        LobbyWatcher lobby(COLOR_BLUE, false);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(30),
            {
                overworld,
                white_dialog,
                arrow_detector,
                big_dialog,
                lobby,
            }
        );
        context.wait_for(100ms);

        //  If we detected the dialog arrow, we need to disambiguate.
        while (ret == 2){
            stream.log("Detected selection arrow.");

            //  Wait for screen to settle.
            context.wait_for(250ms);
            VideoSnapshot screen = stream.video().snapshot();

            if (lobby.detect(screen)){
                ret = 4;
                break;
            }

            arrow_count++;
            ImageStats ore_stats = image_stats(extract_box_reference(screen, ore_box));
            if (is_solid(ore_stats, {0.594724, 0.405276, 0.})){
                stream.log("Need to pay ore.", COLOR_PURPLE);

                arrow_count = 0;
                ImageRGB32 image = to_blackwhite_rgb32_range(
                    extract_box_reference(screen, ore_quantity),
                    true,
                    0xff808080, 0xffffffff
                );
                ImageRGB32 filtered = pad_image(image, 10, 0xffffffff);
                ore.update_with_ocr(stream.logger(), filtered);

                if (ore.quantity < 20){
                    OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::NO_ERROR_REPORT,
                        "You have less than 20 ore. Program stopped. (Quantity: " + ore.to_str() + ")",
                        stream
                    );
                }

                ore_dialog_count++;
                if (ore_dialog_count >= 2){
                    OperationFailedExceptionWithScreenshot::fire(
                        ErrorReportMode::NO_ERROR_REPORT,
                        "Unable to start adventure. Are you out of ore? (Quantity: " + ore.to_str() + ")",
                        stream
                    );
                }
            }else if (arrow_count == 2){
                //  Select a boss.
                stream.log("Detected boss selection.");
                if (boss_slot > 0){
                    for (size_t c = 1; c < boss_slot; c++){
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 400ms);
                    }
                    pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
                }else{
                    pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
                }
            }

            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            break;
        }


        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            continue;

        case 1:
            stream.log("Detected dialog prompt.");
            pbf_press_button(context, BUTTON_B, 160ms, 840ms);
            continue;

        case 2:
            continue;

        case 3:{
            stream.log("Detected save dialog.");
            context.wait_for_all_requests();
            entrance = stream.video().snapshot();
            static int c = 0;
            entrance->save("entrance-" + std::to_string(c++) + ".png");
            pbf_press_button(context, BUTTON_A, 160ms, 5000ms);
            continue;
        }
        case 4:
            stream.log("Detected lobby.");
            return std::move(entrance.frame);

        default:
#if 0
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReportMode::SEND_ERROR_REPORT,
                "obtain_starter(): No recognized state after 30 seconds.",
                stream
            );
#endif
            return std::make_shared<ImageRGB32>();
        }
    }

    return std::make_shared<ImageRGB32>();
}



}
}
}
}
