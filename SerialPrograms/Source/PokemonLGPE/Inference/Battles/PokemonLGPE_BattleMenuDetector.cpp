/*  Battle Menu Detector
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
#include "PokemonLGPE_BattleMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

BattleMenuDetector::BattleMenuDetector(Color color)
    : m_your_box(0.114, 0.916, 0.016, 0.027)
    , m_opponent_box(0.904, 0.048, 0.022, 0.031)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_your_box);
    items.add(COLOR_RED, m_opponent_box);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 left_image = extract_box_reference(screen, m_your_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_opponent_box);

    //Has a bit of a gradient in the box
    //TODO: Do more boxes, horizonal so less gradient variation.
    if (is_solid(left_image, { 0.040, 0.467, 0.493 }) && is_solid(right_image, { 0.074, 0.458, 0.468 })){
        return true;
    }
    return false;
}



}
}
}
