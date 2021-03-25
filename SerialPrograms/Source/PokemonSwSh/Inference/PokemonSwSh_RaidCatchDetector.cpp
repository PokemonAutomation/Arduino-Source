/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/ColorClustering.h"
#include "PokemonSwSh_RaidCatchDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


RaidCatchDetector::RaidCatchDetector(
    VideoFeed& feed,
    std::chrono::milliseconds timeout
)
    : m_feed(feed)
    , m_timeout(timeout)
    , m_box0(feed, 0.82, 0.84 + 0 * 0.078, 0.15, 0.06)
    , m_box1(feed, 0.82, 0.84 + 1 * 0.078, 0.15, 0.06)
    , m_start_time(std::chrono::system_clock::now())
{}
bool RaidCatchDetector::has_timed_out() const{
    return std::chrono::system_clock::now() - m_start_time > m_timeout;
}
bool RaidCatchDetector::detect(){
    QImage image = m_feed.snapshot();
//    cout << "==================" << endl;
    if (!cluster_fit_2(
        extract_box(image, m_box0),
        qRgb(0, 0, 0), 0.90,
        qRgb(255, 255, 255), 0.10
    )){
        return false;
    }
//    cout << "------------------" << endl;
    if (!cluster_fit_2(
        extract_box(image, m_box1),
        qRgb(255, 255, 255), 0.90,
        qRgb(0, 0, 0), 0.10
    )){
        return false;
    }
    return true;
}
bool RaidCatchDetector::wait(ProgramEnvironment& env){
    while (true){
        auto start = std::chrono::system_clock::now();
        env.check_stopping();

        if (has_timed_out()){
            return false;
        }
        if (detect()){
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

