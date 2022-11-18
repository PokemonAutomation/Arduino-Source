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


BattleMenuFinder::BattleMenuFinder(Color color)
    : VisualInferenceCallback("BattleMenuFinder")
    , m_detector(color)
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
        return false;
    }
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}


MoveSelectFinder::MoveSelectFinder(Color color)
    : VisualInferenceCallback("BattleMenuFinder")
    , m_detector(color)
{}
void MoveSelectFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool MoveSelectFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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


TeraCatchFinder::TeraCatchFinder(Color color)
    : VisualInferenceCallback("TeraCatchFinder")
    , m_detector(color)
{}
void TeraCatchFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool TeraCatchFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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
