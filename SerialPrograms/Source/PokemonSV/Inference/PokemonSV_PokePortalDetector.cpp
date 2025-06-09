/*  Poke Portal Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_PokePortalDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


PokePortalDetector::PokePortalDetector(Color color)
    : m_color(color)
    , m_bottom(0.10, 0.94, 0.40, 0.05)
    , m_arrow_union(color, GradientArrowType::RIGHT, {0.025, 0.180, 0.050, 0.080})
    , m_arrow_tera(color, GradientArrowType::RIGHT, {0.025, 0.350, 0.050, 0.080})
    , m_arrow_bottom(color, GradientArrowType::RIGHT, {0.025, 0.460, 0.050, 0.350})
{}
void PokePortalDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    m_arrow_union.make_overlays(items);
    m_arrow_tera.make_overlays(items);
    m_arrow_bottom.make_overlays(items);
}
bool PokePortalDetector::detect(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
    if (!is_solid(bottom, {0.582218, 0.417782, 0.})){
        return false;
    }

    if (m_arrow_union.detect(screen)){
        return true;
    }
    if (m_arrow_tera.detect(screen)){
        return true;
    }
    if (m_arrow_bottom.detect(screen)){
        return true;
    }

    return false;
}
int PokePortalDetector::detect_location(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
    if (!is_solid(bottom, {0.582218, 0.417782, 0.})){
        return -1;
    }

    if (m_arrow_union.detect(screen)){
        return 0;
    }
    if (m_arrow_tera.detect(screen)){
        return 1;
    }

    ImageFloatBox box;
    if (m_arrow_bottom.detect(box, screen)){
//        cout << box.y << endl;
        int slot = (int)((box.y - 0.474074) / 0.0645833 + 0.5);
        if (slot < 0){
            return -1;
        }
        return slot + 2;
    }

    return false;
}



bool PokePortalDetector::move_cursor(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int row
){
    if (row < 0 || row >= 7){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "PokePortalDetector::move_cursor() called with invalid row."
        );
    }

    size_t consecutive_detection_fails = 0;
    size_t moves = 0;
    bool target_reached = false;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int current = this->detect_location(screen);

        //  Failed to detect menu.
        if (current < 0){
            consecutive_detection_fails++;
            if (consecutive_detection_fails > 10){
                dump_image_and_throw_recoverable_exception(
                    info, stream, "UnableToDetectPokePortal",
                    "Unable to detect Poke Portal."
                );
            }
            context.wait_for(std::chrono::milliseconds(50));
            continue;
        }
        consecutive_detection_fails = 0;

        if (moves >= 10){
            stream.log("Unable to move to target after 10 moves.", COLOR_RED);
            return false;
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

        int diff = (7 + current - row) % 7;
        if (diff < 4){
            pbf_press_dpad(context, DPAD_UP, 20, 10);
        }else{
            pbf_press_dpad(context, DPAD_DOWN, 20, 10);
        }
    }
}








}
}
}
