/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLGPE_ShinySymbolDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

ShinySymbolDetector::ShinySymbolDetector(Color color)
    : m_box_star(0.666, 0.779, 0.028, 0.044)
{}
void ShinySymbolDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_star);
}
bool ShinySymbolDetector::read(Logger& logger, const ImageViewRGB32& frame){
    /*
    Shiny (charizard):
    Add infer box: (0.6660, 0.7790, 0.0280, 0.0440), RGB avg [159, 123, 125] avg sum 408 ratio [0.391, 0.301, 0.308] stddev [74.898, 54.696, 53.354] sum 182.948 crop size (54, 48)

    Not shiny (chansey):
    Add infer box: (0.6660, 0.7790, 0.0280, 0.0440), RGB avg [82, 113, 100] avg sum 295 ratio [0.276, 0.384, 0.340] stddev [15.477, 2.178, 2.648] sum 20.303 crop size (54, 48)

    Only had the two to test with for now.
    */

    const auto stats = image_stats(extract_box_reference(frame, m_box_star));
    return stats.stddev.sum() > 100;
}


}
}
}

