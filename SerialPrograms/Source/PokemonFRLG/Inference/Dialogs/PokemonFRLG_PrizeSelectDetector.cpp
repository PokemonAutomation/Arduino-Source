/*  Prize Select Detector
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
#include "PokemonFRLG_PrizeSelectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

PrizeSelectDetector::PrizeSelectDetector(Color color)
    : m_right_box(0.812, 0.726, 0.013, 0.169)
    , m_top_box(0.175, 0.715, 0.649, 0.005)
    , m_bottom_box(0.177, 0.896, 0.645, 0.008)
    , m_selection_box(0.667, 0.514, 0.173, 0.058)
{}
void PrizeSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_right_box);
    items.add(COLOR_RED, m_top_box);
    items.add(COLOR_RED, m_bottom_box);
    items.add(COLOR_RED, m_selection_box);
}
bool PrizeSelectDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(screen, m_bottom_box);
    ImageViewRGB32 selection_image = extract_box_reference(screen, m_selection_box);
    if (is_solid(right_image, { 0.25, 0.38, 0.369 })
        && is_solid(top_image, { 0.25, 0.38, 0.369 })
        && is_solid(bottom_image, { 0.25, 0.38, 0.369 })
        && is_solid(selection_image, { 0.25, 0.38, 0.369 })
        ){
        return true;
    }
    return false;
}


}
}
}
