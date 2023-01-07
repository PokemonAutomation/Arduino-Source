/*  Box Eggs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSV_BoxEggDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BoxEggDetector::BoxEggDetector(Color color)
: m_color(color), m_box{0.659, 0.082, 0.329, 0.043} {}

void BoxEggDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxEggDetector::detect(const ImageViewRGB32& frame) const{
    const auto stats = image_stats(extract_box_reference(frame, m_box));
    return stats.stddev.sum() < 20;
}


}
}
}
