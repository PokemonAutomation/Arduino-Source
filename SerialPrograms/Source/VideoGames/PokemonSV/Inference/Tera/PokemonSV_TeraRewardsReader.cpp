/*  Tera Rewards Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonSV_TeraRewardsReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



SparklyItemDetector::SparklyItemDetector(Color color)
    : VisualInferenceCallback("SparklyItemDetector")
    , m_color(color)
    , m_start_time(WallClock::min())
{
    for (size_t c = 0; c < ITEMS; c++){
        m_boxes[c] = ImageFloatBox{0.070, 0.262 + c*0.0804286, 0.045, 0.080};
        m_sparkly[c] = false;
    }
}
size_t SparklyItemDetector::sparkly_items() const{
    size_t ret = 0;
    for (size_t c = 0; c < ITEMS; c++){
        if (m_sparkly[c]){
            ret++;
        }
    }
    return ret;
}
void SparklyItemDetector::make_overlays(VideoOverlaySet& items) const{
    for (size_t c = 0; c < ITEMS; c++){
        items.add(m_color, m_boxes[c]);
    }
}
bool SparklyItemDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    ImageRGB32 current[ITEMS];
    for (size_t c = 0; c < ITEMS; c++){
        current[c] = filter_rgb32_range(
            extract_box_reference(frame, m_boxes[c]),
            0xff808000, 0xffffffff,
            Color(0xff000000), false
        );
    }

    //  First frame. Save it.
    if (m_start_time == WallClock::min()){
        for (size_t c = 0; c < ITEMS; c++){
            m_last[c] = std::move(current[c]);
        }
        m_start_time = timestamp;
        return false;
    }

    for (size_t c = 0; c < ITEMS; c++){
        if (m_last[c].width() != current[c].width()){
            continue;
        }
        if (m_last[c].height() != current[c].height()){
            continue;
        }
        double rmsd = ImageMatch::pixel_RMSD(m_last[c], current[c]);
//        cout << rmsd << endl;
        m_sparkly[c] |= rmsd > 25;
    }

    return timestamp - m_start_time > std::chrono::seconds(2);
}


size_t SparklyItemDetector::count_sparkly_items(VideoStream& stream, CancellableScope& scope){
    SparklyItemDetector detector;
    wait_until(
        stream, scope, std::chrono::seconds(2),
        {detector}
    );
//    cout << detector.sparkly_items() << endl;
    return detector.sparkly_items();
}







}
}
}
