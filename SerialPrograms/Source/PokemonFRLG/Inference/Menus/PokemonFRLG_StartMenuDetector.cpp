/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG_StartMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

StartMenuDetector::StartMenuDetector(Color color)
    : m_selection_arrow(color, nullptr, MENU_ARROW_BOX)
{}
void StartMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(MENU_ARROW_BOX));
}
bool StartMenuDetector::detect(const ImageViewRGB32& screen){
    return m_selection_arrow.detect(screen);
}


}
}
}
