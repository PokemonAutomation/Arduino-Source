/*  Prize Select Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PrizeSelectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

PrizeSelectDetector::PrizeSelectDetector(Color color)
    : m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
    , m_selection_box(0.705538, 0.528962, 0.212923, 0.0602308)
{}
void PrizeSelectDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_selection_box));
}
bool PrizeSelectDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    ImageViewRGB32 selection_image = extract_box_reference(game_screen, m_selection_box);
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
