/*  Box Empty Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSV_BoxEmptyDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BoxEmptyDetector::BoxEmptyDetector(Color color)
: m_color(color), m_box{0.659, 0.006, 0.329, 0.043} {}

void BoxEmptyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxEmptyDetector::detect(const ImageViewRGB32& frame) const{
    const auto stats = image_stats(extract_box_reference(frame, m_box));
    return stats.stddev.sum() < 100;
}


}
}
}
