/*  Bag Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG_BagDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


BagDetector::BagDetector(Color color)
    : m_bag_shadow_box(0.130769, 0.594471, 0.070192, 0.015865)
    , m_pocket_underline_box(0.043269, 0.137259, 0.279808, 0.005769)
    , m_menu_top_box(0.029808, 0.029086, 0.947115, 0.003606)
    , m_menu_right_box(0.977404, 0.034134, 0.005288, 0.597837)
{}
void BagDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bag_shadow_box));        // Gray   RGB(159, 159, 159)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_pocket_underline_box));  // Orange RGB(255, 152,  50)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));          // Yellow RGB(255, 225,  93)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_box));        // Yellow

}
bool BagDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 bag_shadow_image = extract_box_reference(game_screen, m_bag_shadow_box);
    ImageViewRGB32 pocket_underline_image = extract_box_reference(game_screen, m_pocket_underline_box);
    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_right_image = extract_box_reference(game_screen, m_menu_right_box);


    if (   is_solid(bag_shadow_image,       { 0.333, 0.333, 0.333 }, 0.25, 20)
        && is_solid(pocket_underline_image, { 0.558, 0.333, 0.109 }, 0.25, 20)
        && is_solid(menu_top_image,         { 0.445, 0.393, 0.162 }, 0.25, 20)
        && is_solid(menu_right_image,       { 0.445, 0.393, 0.162 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}



}
}
}
