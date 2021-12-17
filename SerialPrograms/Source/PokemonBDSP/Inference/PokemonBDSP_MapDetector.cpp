/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonBDSP_MapDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


MapDetector::MapDetector()
    : m_box0(0.40, 0.18, 0.10, 0.10)
    , m_box1(0.68, 0.08, 0.06, 0.05)
    , m_box2(0.92, 0.18, 0.06, 0.05)
    , m_box3(0.02, 0.97, 0.12, 0.02)
{
    add_box(m_box0);
    add_box(m_box1);
    add_box(m_box2);
    add_box(m_box3);
}
bool MapDetector::detect(const QImage& frame) const{
    ImageStats stats0 = image_stats(extract_box(frame, m_box0));
    if (!is_solid(stats0, {0.0668203, 0.4447, 0.488479})){
        return false;
    }
    ImageStats stats1 = image_stats(extract_box(frame, m_box1));
    if (!is_solid(stats1, {0.0668203, 0.4447, 0.488479})){
        return false;
    }
    ImageStats stats2 = image_stats(extract_box(frame, m_box2));
    if (!is_solid(stats2, {0.0668203, 0.4447, 0.488479})){
        return false;
    }
    ImageStats stats3 = image_stats(extract_box(frame, m_box3));
    if (!is_solid(stats3, {0.190189, 0.32745, 0.482361})){
        return false;
    }
    return true;
}
bool MapDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}



}
}
}
