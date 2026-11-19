/*  Tutorial Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_TutorialMapDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

TutorialMapDetector::TutorialMapDetector(Color color, VideoOverlay* overlay)
    : m_mission_list(color, ButtonType::ButtonX, {0.005, 0.15, 0.025, 0.11}, overlay)
    , m_close(color, ButtonType::ButtonB, {0.9, 0.94, 0.04, 0.04}, overlay)
{}

void TutorialMapDetector::make_overlays(VideoOverlaySet& items) const{
    m_mission_list.make_overlays(items);
    m_close.make_overlays(items);
}

bool TutorialMapDetector::detect(const ImageViewRGB32& screen){
    return m_mission_list.detect(screen) && m_close.detect(screen);
}

}
}
}
