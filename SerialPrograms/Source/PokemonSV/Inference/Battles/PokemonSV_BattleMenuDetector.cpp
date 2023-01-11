/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
//#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSV_BattleMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_callouts_button(color, WhiteButton::ButtonMinus, {0.35, 0.87, 0.30, 0.05})
    , m_arrow(color, GradientArrowType::RIGHT, {0.75, 0.62, 0.05, 0.35})
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_callouts_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen) const{
    if (!m_callouts_button.detect(screen)){
//        cout << "status button" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}
int8_t BattleMenuDetector::detect_slot(const ImageViewRGB32& screen) const{
    if (!m_callouts_button.detect(screen)){
//        cout << "status button" << endl;
        return -1;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
        return -1;
    }

    double y = box.y + box.height * 0.5;

    return (int8_t)((y - 0.761111) / 0.0814815 + 0.5);
}




MoveSelectDetector::MoveSelectDetector(Color color)
    : m_info_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.705, 0.550, 0.050, 0.410})
{}
void MoveSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_info_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool MoveSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (!m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
//        cout << "arrow" << endl;
        return false;
    }
    return true;
}
int8_t MoveSelectDetector::detect_slot(const ImageViewRGB32& screen) const{
    if (!m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
//        cout << "arrow" << endl;
        return false;
    }

    double y = box.y + box.height * 0.5;
//    cout << "y = " << y << endl;

    return (int8_t)((y - 0.602778) / 0.103549 + 0.5);
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


TargetSelectDetector::TargetSelectDetector(Color color)
    : m_opponent(color, GradientArrowType::DOWN, {0.45, 0.07, 0.10, 0.10})
    , m_player0(color, GradientArrowType::DOWN, {0.20, 0.46, 0.10, 0.10})
    , m_player1(color, GradientArrowType::DOWN, {0.37, 0.46, 0.10, 0.10})
    , m_player2(color, GradientArrowType::DOWN, {0.53, 0.46, 0.10, 0.10})
    , m_player3(color, GradientArrowType::DOWN, {0.70, 0.46, 0.10, 0.10})
{}
void TargetSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_opponent.make_overlays(items);
    m_player0.make_overlays(items);
    m_player1.make_overlays(items);
    m_player2.make_overlays(items);
    m_player3.make_overlays(items);
}
bool TargetSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (m_opponent.detect(screen)){
        return true;
    }
    if (m_player0.detect(screen)){
        return true;
    }
    if (m_player1.detect(screen)){
        return true;
    }
    if (m_player2.detect(screen)){
        return true;
    }
    if (m_player3.detect(screen)){
        return true;
    }
    return false;
}
int8_t TargetSelectDetector::detect_slot(const ImageViewRGB32& screen) const{
    if (m_opponent.detect(screen)){
        return 0;
    }
    if (m_player0.detect(screen)){
        return 1;
    }
    if (m_player1.detect(screen)){
        return 2;
    }
    if (m_player2.detect(screen)){
        return 3;
    }
    if (m_player3.detect(screen)){
        return 4;
    }
    return -1;
}





TeraCatchDetector::TeraCatchDetector(Color color)
    : m_color(color)
    , m_callouts_button(color, WhiteButton::ButtonMinus, {0.35, 0.87, 0.30, 0.05})
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
//        cout << "button and yellow bad" << endl;
        return false;
    }

    if (!m_arrow.detect(screen)){
//        cout << "arrow bad" << endl;
        return false;
    }

    if (m_callouts_button.detect(screen)){
        return false;
    }

    return true;
}









}
}
}
