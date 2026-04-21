/*  Map Detector
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
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_MapDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

KantoMapDetector::KantoMapDetector(Color color)
    : m_top_box(0.010, 0.010, 0.500, 0.080) // blue (0, 122, 255)
    , m_left_box(0.069, 0.110, 0.025, 0.880) // white
    , m_right_box(0.902, 0.110, 0.025, 0.880) // white
    , m_land_box(0.105, 0.600, 0.087, 0.028) // green (31, 173, 0)
    // , m_route_25_box(0.603, 0.252, 0.090, 0.040) // orange (255, 176, 0)
{}
void KantoMapDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_land_box));
}
bool KantoMapDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 land_image = extract_box_reference(game_screen, m_land_box);
    if (is_solid(top_image, { 0.0000, 0.3236, 0.6764 }, 0.25, 20)
        && is_white(left_image)
        && is_white(right_image)
        && is_solid(land_image, { 0.1520, 0.8480, 0.0000 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}



}
}
}
