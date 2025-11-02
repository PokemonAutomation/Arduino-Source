/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLZA_MainMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


MainMenuDetector::MainMenuDetector(Color color, VideoOverlay* overlay)
    : m_right_arrow_button(color, ButtonType::ButtonRight, {0.382, 0.081, 0.029, 0.056}, overlay)
    , m_b_button(color, ButtonType::ButtonB, {0.87, 0.940, 0.077, 0.044}, overlay)
{}

void MainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_right_arrow_button.make_overlays(items);
    m_b_button.make_overlays(items);
}

bool MainMenuDetector::detect(const ImageViewRGB32& screen){
    return m_right_arrow_button.detect(screen) && m_b_button.detect(screen);
}




}
}
}
