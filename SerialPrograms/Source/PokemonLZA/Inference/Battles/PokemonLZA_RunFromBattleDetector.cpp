/*  Run From Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_RunFromBattleDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




RunFromBattleDetector::RunFromBattleDetector(Color color, VideoOverlay* overlay)
    : m_button(
        color, ButtonType::ButtonMinus,
        {0.004456, 0.635499, 0.065062, 0.077655},
        overlay
    )
{}
void RunFromBattleDetector::make_overlays(VideoOverlaySet& items) const{
    m_button.make_overlays(items);
}
bool RunFromBattleDetector::detect(const ImageViewRGB32& screen){
    return m_button.detect(screen);
}






}
}
}
