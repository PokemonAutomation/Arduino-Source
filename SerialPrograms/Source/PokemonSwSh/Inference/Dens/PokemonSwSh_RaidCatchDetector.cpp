/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_RaidCatchDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


RaidCatchDetector::RaidCatchDetector(
    VideoOverlay& overlay
)
    : m_left0 (overlay, 0.82, 0.85 + 0 * 0.078, 0.01, 0.04)
    , m_right0(overlay, 0.96, 0.85 + 0 * 0.078, 0.01, 0.04)
    , m_left1 (overlay, 0.82, 0.85 + 1 * 0.078, 0.01, 0.04)
    , m_right1(overlay, 0.96, 0.85 + 1 * 0.078, 0.01, 0.04)
    , m_text0 (overlay, 0.82, 0.84 + 0 * 0.078, 0.15, 0.06)
    , m_text1 (overlay, 0.82, 0.84 + 1 * 0.078, 0.15, 0.06)
    , m_start_time(std::chrono::system_clock::now())
{}
bool RaidCatchDetector::has_timed_out(std::chrono::milliseconds timeout) const{
    return std::chrono::system_clock::now() - m_start_time > timeout;
}
bool RaidCatchDetector::detect(const QImage& screen){
    ImageStats left0 = image_stats(extract_box(screen, m_left0));
    if (!is_black(left0)){
        return false;
    }
    ImageStats right0 = image_stats(extract_box(screen, m_right0));
    if (!is_black(right0)){
        return false;
    }
    if (euclidean_distance(left0.average, right0.average) > 10) return false;
    ImageStats left1 = image_stats(extract_box(screen, m_left1));
    if (!is_white(left1)){
        return false;
    }
    ImageStats right1 = image_stats(extract_box(screen, m_right1));
    if (!is_white(right1)){
        return false;
    }
    if (euclidean_distance(left1.average, right1.average) > 10) return false;



//    cout << "==================" << endl;
    if (!cluster_fit_2(
        extract_box(screen, m_text0),
        qRgb(0, 0, 0), 0.90,
        qRgb(255, 255, 255), 0.10
    )){
        return false;
    }
//    cout << "------------------" << endl;
    if (!cluster_fit_2(
        extract_box(screen, m_text1),
        qRgb(255, 255, 255), 0.90,
        qRgb(0, 0, 0), 0.10
    )){
        return false;
    }
    return true;
}
bool RaidCatchDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}
bool RaidCatchDetector::wait(
    ProgramEnvironment& env,
    VideoFeed& feed,
    std::chrono::milliseconds timeout
){
    while (true){
        auto start = std::chrono::system_clock::now();
        env.check_stopping();

        if (has_timed_out(timeout)){
            return false;
        }
        if (detect(feed.snapshot())){
            return true;
        }

        auto end = std::chrono::system_clock::now();
        auto duration = end - start;
        if (duration < std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - duration);
        }
    }
}



}
}
}

