/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_StartBattleDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



StartBattleDetector::StartBattleDetector(
    VideoFeed& feed,
    std::chrono::milliseconds timeout
)
    : m_feed(feed)
    , m_timeout(timeout)
    , m_box(feed, 0.65, 0.89, 0.25, 0.07)
    , m_start_time(std::chrono::system_clock::now())
{}

bool StartBattleDetector::has_timed_out() const{
    return std::chrono::system_clock::now() - m_start_time > m_timeout;
}
bool StartBattleDetector::detect(){
    QImage image = extract_box(m_feed.snapshot(), m_box);
    ImageStats stats = pixel_stats(image);
//    cout << "mean = " << stats.average << ", stddev = " << stats.stddev << endl;
//        return stats.stddev.sum() < 10;

    //  White screen.
        if (stats.average.sum() > 600 && stats.stddev.sum() < 10){
            return true;
        }

    //  Grey text box.
    if (stats.average.sum() < 200 && stats.stddev.sum() < 10){
        return true;
    }

    return false;
}



}
}
}
