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
#include "PokemonHome_SummaryScreenDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


SummaryScreenDetector::SummaryScreenDetector(VideoOverlay* overlay) 
    : m_button_B_detector(COLOR_BLACK, ButtonType::ButtonB, { 0.100, 0.956, 0.107, 0.041 }, overlay){}

void SummaryScreenDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_B_detector.make_overlays(items);
}

bool SummaryScreenDetector::detect(const ImageViewRGB32& screen){
    return m_button_B_detector.detect(screen);
}


}
}
}
