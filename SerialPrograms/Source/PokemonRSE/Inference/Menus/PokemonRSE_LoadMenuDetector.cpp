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
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_LoadMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

LoadMenuDetector::LoadMenuDetector(Color color)
    : m_right_box(0.975, 0.02, 0.015, 0.96)
    , m_left_box(0.01, 0.02, 0.015, 0.96)
    , m_save_box(0.751685, 0.053648, 0.167558, 0.05736)
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

    //Ruby and Sapphire
    if ((is_solid(right_image, { 0.254, 0.274576, 0.471186 }, 0.25, 2) //75, 81, 139
        && is_solid(left_image, { 0.254, 0.274576, 0.471186 }, 0.25, 2)
        && is_white(save_image)) //white
        ||
        (is_solid(right_image, { 0.259259, 0.27037, 0.47037 }, 0.25, 2) //Emerald
         && is_solid(right_image, { 0.259259, 0.27037, 0.47037 }, 0.25, 2) //140, 146, 254
         && is_white(save_image)
        )
    ){
        return true;
    }
    return false;
}


}
}
}
