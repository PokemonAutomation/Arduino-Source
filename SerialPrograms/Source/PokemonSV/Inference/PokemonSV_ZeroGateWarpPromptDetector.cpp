/*  Zero Gate Warp Prompt Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_ZeroGateWarpPromptDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ZeroGateWarpPromptDetector::ZeroGateWarpPromptDetector(Color color)
    : m_arrow(color, GradientArrowType::RIGHT, {0.5, 0.32, 0.40, 0.39})
{}

void ZeroGateWarpPromptDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow.make_overlays(items);
}
bool ZeroGateWarpPromptDetector::detect(const ImageViewRGB32& screen){
    return detect_location(screen) >= 0;
}
int ZeroGateWarpPromptDetector::detect_location(const ImageViewRGB32& screen) const{
    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
        return -1;
    }

    int index = (int)((box.y - 0.337037) / 0.074074 + 0.5);

//    cout << index << endl;

    return index;
}
bool ZeroGateWarpPromptDetector::move_cursor(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int row
) const{
    VideoOverlaySet overlays(stream.overlay());
    make_overlays(overlays);

    size_t consecutive_detection_fails = 0;
    size_t moves = 0;
    bool target_reached = false;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int current = this->detect_location(screen);

        stream.log("Current Location: " + std::to_string(current));

        if (current < 0){
            consecutive_detection_fails++;
            if (consecutive_detection_fails > 10){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "ZeroGateWarpPromptDetector::move_cursor(): Unable to detect cursor.",
                    stream,
                    screen
                );
            }
            context.wait_for(std::chrono::milliseconds(50));
            continue;
        }
        consecutive_detection_fails = 0;

        if (moves >= 10){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to move to target after 10 moves.",
                stream,
                screen
            );
        }

        //  We're done!
        if (current == row){
            if (target_reached){
                return true;
            }else{
                //  Don't return yet. Wait a second to make sure the video is
                //  in steady state before we return.
                target_reached = true;
                context.wait_for(std::chrono::seconds(1));
                continue;
            }
        }
        target_reached = false;

        moves++;

        const int rows = 5;
        int diff = (rows + current - row) % rows;
        if (diff < 3){
            for (int c = 0; c < diff - 1; c++){
                pbf_press_dpad(context, DPAD_UP, 10, 10);
            }
            pbf_press_dpad(context, DPAD_UP, 20, 30);
        }else{
            diff = rows - diff;
            for (int c = 0; c < diff - 1; c++){
                pbf_press_dpad(context, DPAD_DOWN, 10, 10);
            }
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
        }
    }
}




}
}
}
