/*  Max Lair Enter Lobby
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh_MaxLair_Run_EnterLobby.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



class GreyDialogDetector : public VisualInferenceCallback{
public:
    GreyDialogDetector()
        : VisualInferenceCallback("GreyDialogDetector")
        , m_box0(0.180, 0.815, 0.015, 0.030)
        , m_box1(0.785, 0.840, 0.030, 0.050)
    {}

    bool detect(const ImageViewRGB32& screen){
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
    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(COLOR_RED, m_box0);
        items.add(COLOR_RED, m_box1);
    }
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        return detect(frame);
    }

private:
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
};




std::shared_ptr<const ImageRGB32> enter_lobby(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t boss_slot, bool connect_to_internet,
    ReadableQuantity999& ore
){
    pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);

    if (connect_to_internet){
        connect_to_internet_with_inference(info, stream, context);
    }

    VideoOverlaySet boxes(stream.overlay());
    SelectionArrowFinder arrow_detector(stream.overlay(), ImageFloatBox(0.350, 0.450, 0.500, 0.400));
    GreyDialogDetector dialog_detector;
    arrow_detector.make_overlays(boxes);
    dialog_detector.make_overlays(boxes);

//    OverlayBoxScope ore_box(stream.overlay(), {0.900, 0.015, 0.020, 0.040});
    OverlayBoxScope ore_box(stream.overlay(), {0.930, 0.050, 0.065, 0.010});
    OverlayBoxScope ore_quantity(stream.overlay(), {0.945, 0.010, 0.0525, 0.050});

    size_t presses = 0;
    size_t arrow_count = 0;
    size_t ore_dialog_count = 0;
    while (presses < 50){
        presses++;
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();

        VideoSnapshot screen = stream.video().snapshot();
        if (!arrow_detector.detect(screen)){
            continue;
        }

        arrow_count++;

        stream.log("Detected dialog prompt.");
//        screen.save("test.png");

        //  We need to pay ore.
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
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    "You have less than 20 ore. Program stopped. (Quantity: " + ore.to_str() + ")",
                    stream
                );
            }

            ore_dialog_count++;
            if (ore_dialog_count >= 2){
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    "Unable to start adventure. Are you out of ore? (Quantity: " + ore.to_str() + ")",
                    stream
                );
            }

            continue;
        }

        //  Detected save dialog.
        if (dialog_detector.detect(screen)){
            stream.log("Detected save dialog.");
            context.wait_for_all_requests();
            VideoSnapshot entrance = stream.video().snapshot();
            pbf_press_button(context, BUTTON_A, 10, 5 * TICKS_PER_SECOND);
            context.wait_for_all_requests();
            return std::move(entrance.frame);
        }

        //  Select a boss.
        if (arrow_count == 2){
            stream.log("Detected boss selection.");
            if (boss_slot > 0){
                for (size_t c = 1; c < boss_slot; c++){
                    pbf_press_dpad(context, DPAD_DOWN, 10, 50);
                }
                pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            }else{
                pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
            }
        }
    }

    return std::make_shared<ImageRGB32>();
}



}
}
}
}
