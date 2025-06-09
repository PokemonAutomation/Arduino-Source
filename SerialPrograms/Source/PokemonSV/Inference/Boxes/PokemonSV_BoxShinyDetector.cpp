/*  Box Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSV_BoxShinyDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BoxShinyDetector::BoxShinyDetector(Color color, const ImageFloatBox& box)
: m_color(color), m_box(box) {}

void BoxShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxShinyDetector::detect(const ImageViewRGB32& frame){
    const auto stats = image_stats(extract_box_reference(frame, m_box));
    // cout << "stats.stddev.sum() " << stats.stddev.sum() << endl;
    // On screenshots collected from macOS-Mirabox, non-shiny has stddev of at most 3.0, while
    // shiny has stdddev of 160.
    return stats.stddev.sum() > 100;
}


}
}
}
