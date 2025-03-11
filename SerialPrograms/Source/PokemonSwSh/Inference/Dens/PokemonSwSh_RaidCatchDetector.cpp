/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ColorClustering.h"
#include "PokemonSwSh_RaidCatchDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


RaidCatchDetector::RaidCatchDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("RaidCatchDetector")
    , m_left0 (0.82, 0.85 + 0 * 0.078, 0.01, 0.04)
    , m_right0(0.96, 0.85 + 0 * 0.078, 0.01, 0.04)
//    , m_left1 (0.82, 0.85 + 1 * 0.078, 0.01, 0.04)
//    , m_right1(0.96, 0.85 + 1 * 0.078, 0.01, 0.04)
    , m_text0 (0.82, 0.84 + 0 * 0.078, 0.15, 0.06)
    , m_text1 (0.82, 0.84 + 1 * 0.078, 0.15, 0.06)
    , m_arrow(overlay, ImageFloatBox(0.75, 0.82, 0.10, 0.10))
{}
void RaidCatchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left0);
    items.add(COLOR_RED, m_right0);
//    items.add(COLOR_RED, m_left1);
//    items.add(COLOR_RED, m_right1);
    items.add(COLOR_RED, m_text0);
    items.add(COLOR_RED, m_text1);
    m_arrow.make_overlays(items);
}
bool RaidCatchDetector::detect(const ImageViewRGB32& screen){
    ImageStats left0 = image_stats(extract_box_reference(screen, m_left0));
    if (!is_black(left0)){
        return false;
    }
    ImageStats right0 = image_stats(extract_box_reference(screen, m_right0));
    if (!is_black(right0)){
        return false;
    }
    if (euclidean_distance(left0.average, right0.average) > 10) return false;
#if 0
    ImageStats left1 = image_stats(extract_box_reference(screen, m_left1));
    if (!is_white(left1)){
        return false;
    }
    ImageStats right1 = image_stats(extract_box_reference(screen, m_right1));
    if (!is_white(right1)){
        return false;
    }
    if (euclidean_distance(left1.average, right1.average) > 10) return false;
#endif

//    cout << "==================" << endl;
    if (!cluster_fit_2(
        extract_box_reference(screen, m_text0),
        Color(0, 0, 0), 0.90,
        Color(255, 255, 255), 0.10
    )){
        return false;
    }
//    cout << "------------------" << endl;
    if (!cluster_fit_2(
        extract_box_reference(screen, m_text1),
        Color(255, 255, 255), 0.90,
        Color(0, 0, 0), 0.10
    )){
        return false;
    }

    if (!m_arrow.detect(screen)){
        return false;
    }

    return true;
}
bool RaidCatchDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    return m_trigger_count >= 5;
}




}
}
}

