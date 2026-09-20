/*  Tera Rewards Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_TeraRewardsMenu.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraRewardsMenuDetector::TeraRewardsMenuDetector(Color color)
    : m_color(color)
    , m_next_button(
        COLOR_CYAN,
        WhiteButton::ButtonA,
        {0.8, 0.93, 0.2, 0.07}
    )
    , m_arrow(color, GradientArrowType::RIGHT, {0, 0, 1, 1})
{}
void TeraRewardsMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_next_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool TeraRewardsMenuDetector::detect(const ImageViewRGB32& screen){
    if (!m_next_button.detect(screen)){
        return false;
    }
    if (m_arrow.detect(screen)){
        return false;
    }
    return true;
}




}
}
}
