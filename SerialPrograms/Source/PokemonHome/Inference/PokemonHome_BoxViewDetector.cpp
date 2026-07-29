/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_BoxViewDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

BoxViewDetector::BoxViewDetector(VideoOverlay* overlay) : m_button_plus_detector(COLOR_BLACK, ButtonType::ButtonPlus, { 0.100, 0.956, 0.107, 0.041 }, overlay){}

void BoxViewDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_plus_detector.make_overlays(items);
}

bool BoxViewDetector::detect(const ImageViewRGB32& screen){
    return m_button_plus_detector.detect(screen);
}

}
}
}
