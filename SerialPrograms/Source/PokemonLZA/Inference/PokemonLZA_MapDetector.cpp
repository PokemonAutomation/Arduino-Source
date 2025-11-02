/* Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_MapDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



MapDetector::MapDetector(Color color, VideoOverlay* overlay)
    : m_b_button(
        color,
        ButtonType::ButtonB,
        {0.760730, 0.937023, 0.241416, 0.064885},
        overlay
    )
{}

void MapDetector::make_overlays(VideoOverlaySet& items) const{
    m_b_button.make_overlays(items);
}

bool MapDetector::detect(const ImageViewRGB32& screen){
    return m_b_button.detect(screen);
}




}
}
}
