/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonFRLG_StartMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

StartMenuDetector::StartMenuDetector(Color color)
    : m_right_box(0.864, 0.750, 0.006, 0.173)
    , m_top_box(0.128, 0.744, 0.743, 0.006)
    , m_bottom_box(0.128, 0.917, 0.736, 0.006)
{}
void StartMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_right_box);
    items.add(COLOR_RED, m_top_box);
    items.add(COLOR_RED, m_bottom_box);
}
bool StartMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(screen, m_bottom_box);
    if (is_solid(right_image, { 0.00, 0.38, 0.62 })
        && is_solid(top_image, { 0.00, 0.38, 0.62 })
        && is_solid(bottom_image, { 0.00, 0.38, 0.62 })
        ){
        return true;
    }
    return false;
}


}
}
}
