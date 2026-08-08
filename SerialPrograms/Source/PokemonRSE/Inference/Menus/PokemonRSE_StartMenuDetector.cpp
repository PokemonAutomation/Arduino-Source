/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_StartMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

StartMenuDetector::StartMenuDetector(Color color)
    : m_menu_top_box(0.766756, 0.046978, 0.197701, 0.01334)
    , m_menu_bottom_box(0.766756, 0.94, 0.197701, 0.01334)
{}
void StartMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_bottom_box));
}
bool StartMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_bottom_image = extract_box_reference(game_screen, m_menu_bottom_box);

    return (is_white(menu_top_image)
        && is_white(menu_bottom_image)
    );
}


}
}
}
