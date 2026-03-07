/*  Load Menu Detector
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
#include "PokemonFRLG_LoadMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LoadMenuDetector::LoadMenuDetector(Color color)
    : m_right_box(0.941846, 0.0190769, 0.0516923, 0.960577)
    , m_left_box(0.00646154, 0.0180385, 0.0566154, 0.961615)
    , m_save_box(0.836, 0.0595769, 0.0578462, 0.0747692)
{}
void LoadMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_BLUE, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_BLUE, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_BLUE, GAME_BOX.inner_to_outer(m_save_box));
}
bool LoadMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 save_image = extract_box_reference(game_screen, m_save_box);

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
