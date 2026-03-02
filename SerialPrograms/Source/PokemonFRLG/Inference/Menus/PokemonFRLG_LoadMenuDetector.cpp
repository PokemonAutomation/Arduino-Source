/*  Load Menu Detector
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
#include "PokemonFRLG_LoadMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LoadMenuDetector::LoadMenuDetector(Color color)
    : m_right_box(0.859, 0.023, 0.042, 0.925)
    , m_left_box(0.099, 0.022, 0.046, 0.926)
    , m_save_box(0.773, 0.062, 0.047, 0.072)
{}
void LoadMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_right_box);
    items.add(COLOR_BLUE, m_left_box);
    items.add(COLOR_BLUE, m_save_box);
}
bool LoadMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 save_image = extract_box_reference(screen, m_save_box);

    if (is_solid(right_image, { 0.244, 0.282, 0.474 }) //blue
        && is_solid(left_image, { 0.244, 0.282, 0.474 }) //blue
        && is_solid(save_image, { 0.25, 0.38, 0.369 }) //white
        ){
        return true;
    }
    return false;
}


}
}
}
