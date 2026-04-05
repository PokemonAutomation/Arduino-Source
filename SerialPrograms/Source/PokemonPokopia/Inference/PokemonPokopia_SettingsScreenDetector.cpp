/*  Settings Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonPokopia_SettingsScreenDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{


SettingsScreenDetector::SettingsScreenDetector(ImageFloatBox box)
    : m_box(box)
{}

void SettingsScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_WHITE, m_box);
}

bool SettingsScreenDetector::detect(const ImageViewRGB32& screen){
    const ImageStats stats = image_stats(extract_box_reference(screen, m_box));
    if (!is_solid(stats, {0.427, 0.397, 0.176})){
        return false;
    }
    return true;
}


}
}
}
