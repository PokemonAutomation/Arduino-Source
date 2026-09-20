/*  Moves Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonPokopia_MovesDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



OverworldDetector::OverworldDetector(
    Color color,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_overlay(overlay)
{}
void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, MOVES_LEFT_ARROW_BOX);
    items.add(m_color, MOVES_RIGHT_ARROW_BOX);
}
bool OverworldDetector::detect(const ImageViewRGB32& screen){
    ButtonDetector moves_left_detector(m_color, ButtonType::ButtonDpadLeft, MOVES_LEFT_ARROW_BOX, m_overlay);
    ButtonDetector moves_right_detector(m_color, ButtonType::ButtonDpadRight, MOVES_RIGHT_ARROW_BOX, m_overlay);

    bool found = moves_left_detector.detect(screen) && moves_right_detector.detect(screen);

    return found;
}



}
}
}
