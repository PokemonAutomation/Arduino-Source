/*  Max Lair Detect Item Select Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


ItemSelectDetector::ItemSelectDetector(VideoOverlay& overlay, bool stop_no_detect)
    : m_stop_on_no_detect(stop_no_detect)
    , m_bottom_main(overlay, 0.100, 0.970, 0.700, 0.020, Qt::blue)
    , m_bottom_right(overlay, 0.920, 0.970, 0.070, 0.020, Qt::blue)
    , m_blue(overlay, 0.600, 0.020, 0.200, 0.060, Qt::blue)
{}

bool ItemSelectDetector::detect(const QImage& screen) const{
    ImageStats bottom_main = image_stats(extract_box(screen, m_bottom_main));
//    cout << bottom_main.average << ", " << bottom_main.stddev << endl;
    if (!is_black(bottom_main)){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box(screen, m_bottom_right));
//    cout << bottom_right.average << ", " << bottom_right.stddev << endl;
    if (bottom_right.stddev.sum() < 30){
        return false;
    }
    ImageStats blue = image_stats(extract_box(screen, m_blue));
//    cout << blue.average << ", " << blue.stddev << endl;
    if (!is_solid(blue, {0.0286572, 0.40799, 0.563353})){
        return false;
    }
    return true;
}
bool ItemSelectDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}


}
}
}
}
