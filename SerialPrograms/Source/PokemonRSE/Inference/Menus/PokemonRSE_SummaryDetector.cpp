/*  Summary Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_SummaryDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

SummaryDetector::SummaryDetector(Color color)
    : m_top_bar_box(0.639093, 0.014963, 0.014185, 0.058694)
    , m_page_1_box(0.389085, 0.03764, 0.007979, 0.014674) //white
    , m_page_2_box(0.456, 0.03764, 0.007979, 0.014674)
    , m_page_3_box(0.523, 0.03764, 0.007979, 0.014674)
    , m_page_4_box(0.589, 0.03764, 0.007979, 0.014674)
{}
void SummaryDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_4_box));
}
bool SummaryDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 top_bar_image = extract_box_reference(game_screen, m_top_bar_box);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    ImageViewRGB32 page_4_image = extract_box_reference(game_screen, m_page_4_box);

    if ((is_solid(top_bar_image, { 0.3798, 0.37674, 0.24341 }, 0.25, 20) //RS light yellow 245, 243, 157
        ||is_solid(top_bar_image, { 0.34652, 0.3083, 0.345156889495 }, 0.25, 20)) //E light purple 254, 226, 253
        && is_white(page_1_image)
        && !is_white(page_2_image)
        && !is_white(page_3_image)
        && !is_white(page_4_image)
    ){
        return true;
    }
    return false;
}


}
}
}
