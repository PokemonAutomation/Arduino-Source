/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSV_BattleMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_status_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.75, 0.62, 0.05, 0.18})
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
    : m_status_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.705, 0.550, 0.050, 0.410})
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



TerastallizingDetector::TerastallizingDetector(Color color)
    : m_color(color)
    , m_box(0.6, 0.7, 0.06, 0.14)
{}
void TerastallizingDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool TerastallizingDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats box = image_stats(extract_box_reference(screen, m_box));
    return box.stddev.sum() > 240;
}

TeraCatchDetector::TeraCatchDetector(Color color)
    : m_color(color)
    , m_button(0.801, 0.818, 0.005, 0.047)
    , m_box_right(0.95, 0.81, 0.02, 0.06)
    , m_arrow(color, GradientArrowType::RIGHT, {0.75, 0.80, 0.08, 0.09})
{}
void TeraCatchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_right);
    m_arrow.make_overlays(items);
}
bool TeraCatchDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats button = image_stats(extract_box_reference(screen, m_button));
//    cout << button.average << button.stddev << endl;
//    extract_box_reference(screen, m_button).save("temp.png");

    ImageStats yellow = image_stats(extract_box_reference(screen, m_box_right));
//    cout << box.average << box.stddev << endl;

    bool button_ok = is_solid(button, {0.117281, 0.311767, 0.570951}, 0.20, 20) || is_black(button, 100, 15);
    bool yellow_ok = is_solid(yellow, {0.554348, 0.445652, 0.}, 0.15, 20);
    if (!button_ok && !yellow_ok){
        return false;
    }

    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}









}
}
}
