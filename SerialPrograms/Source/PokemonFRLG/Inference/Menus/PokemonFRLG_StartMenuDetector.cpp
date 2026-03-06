/*  Start Menu Detector
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
#include "PokemonFRLG_StartMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

StartMenuDetector::StartMenuDetector(Color color)
    : m_right_box(0.989846, 0.775077, 0.00369231, 0.2025)
    , m_top_box(0.004, 0.769885, 0.989538, 0.00519231)
    , m_bottom_box(0.00153846, 0.972385, 0.988308, 0.00623077)
{}
void StartMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool StartMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    if (is_solid(right_image, { 0.00, 0.38, 0.62 })
        && is_solid(top_image, { 0.00, 0.38, 0.62 })
        && is_solid(bottom_image, { 0.00, 0.38, 0.62 })
    ){
        return true;
    }
    return false;
}


}
}
}
