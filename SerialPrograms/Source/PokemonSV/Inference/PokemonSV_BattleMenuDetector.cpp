/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSV_BattleMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_status_button(WhiteButton::ButtonY, ImageFloatBox(0.35, 0.90, 0.30, 0.08), color)
    , m_arrow(ImageFloatBox(0.75, 0.62, 0.05, 0.18), color)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_status_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen) const{
    if (!m_status_button.detect(screen)){
        return false;
    }
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}




MoveSelectDetector::MoveSelectDetector(Color color)
    : m_status_button(WhiteButton::ButtonY, ImageFloatBox(0.35, 0.90, 0.30, 0.08), color)
    , m_arrow(ImageFloatBox(0.705, 0.550, 0.050, 0.410), color)
{}
void MoveSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_status_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool MoveSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (!m_status_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
//        cout << "arrow" << endl;
        return false;
    }
    return true;
}





TeraCatchDetector::TeraCatchDetector(Color color)
    : m_arrow(ImageFloatBox(0.75, 0.80, 0.08, 0.09), color)
{}
void TeraCatchDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow.make_overlays(items);
}
bool TeraCatchDetector::detect(const ImageViewRGB32& screen) const{
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}









}
}
}
