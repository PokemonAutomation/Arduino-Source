/*  No Minimap Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_NoMinimapDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



NoMinimapDetector::NoMinimapDetector(Logger& logger, Color color)
    : m_color(color)
    , m_ball(0.890, 0.800, 0.030, 0.060)
    , m_overworld(color)
{}
void NoMinimapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_ball);
}
bool NoMinimapDetector::detect(const ImageViewRGB32& screen) const{
    return !m_overworld.detect(screen);
}




}
}
}
