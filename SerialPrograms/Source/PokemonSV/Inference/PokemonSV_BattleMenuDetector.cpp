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


BattleMenuDetector::BattleMenuDetector()
    : m_status_button(WhiteButton::ButtonY, ImageFloatBox(0.35, 0.90, 0.30, 0.08))
    , m_arrow(ImageFloatBox(0.7, 0.6, 0.2, 0.38))
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


BattleMenuFinder::BattleMenuFinder()
    : VisualInferenceCallback("BattleMenuFinder")
{}

void BattleMenuFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool BattleMenuFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!m_detector.detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    bool detected = m_trigger_count >= 5;
    if (detected){
//        cout << "Detected Battle Menu" << endl;
    }
    return detected;
}




}
}
}
