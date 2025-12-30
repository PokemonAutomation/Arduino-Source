/*  Flavor Power Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLZA_FlavorPowerScreenDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


FlavorPowerScreenDetector::FlavorPowerScreenDetector()
    : m_top_pink_area(0.184, 0.019, 0.030, 0.041)
    , m_left_white_area(0.044, 0.238, 0.021, 0.143)
    , m_bottom_white_area(0.105, 0.459, 0.090, 0.036)
    , m_donut_area(0.077, 0.182, 0.149, 0.252)
{}

void FlavorPowerScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_top_pink_area);
    items.add(COLOR_WHITE, m_left_white_area);
    items.add(COLOR_WHITE, m_bottom_white_area);
}

bool FlavorPowerScreenDetector::detect(const ImageViewRGB32& screen){
    // Check box 1
    const ImageStats stats1 = image_stats(extract_box_reference(screen, m_top_pink_area));
    if (!is_solid(stats1, {0.359, 0.328, 0.314})){
        return false;
    }

    // Check box 2
    const ImageStats stats2 = image_stats(extract_box_reference(screen, m_left_white_area));
    if (!is_solid(stats2, {0.341, 0.340, 0.319})){
        return false;
    }

    // Check box 3
    const ImageStats stats3 = image_stats(extract_box_reference(screen, m_bottom_white_area));
    if (!is_solid(stats3, {0.341, 0.340, 0.319})){
        return false;
    }

    const ImageStats stats4 = image_stats(extract_box_reference(screen, m_donut_area));
    if (stats4.stddev.sum() < 50.0){
        return false;
    }

    return true;
}


}
}
}
