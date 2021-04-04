/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool is_dialog_grey(const ImageStats& stats){
//    cout << "stddev = " << stats.stddev.sum() << endl;
//    cout << "average = " << stats.average << endl;

    if (stats.average.sum() > 200 || stats.stddev.sum() > 5){
        return false;
    }

    FloatPixel color_ratio = stats.average / stats.average.sum();
    double distance = euclidean_distance(
        color_ratio,
        FloatPixel(0.337067, 0.325728, 0.337204)
    );

    return distance < 0.1;
}
bool is_dialog_grey(const QImage& image){
    return is_dialog_grey(pixel_stats(image));
}



StartBattleDetector::StartBattleDetector(
    VideoFeed& feed,
    std::chrono::milliseconds timeout
)
    : m_feed(feed)
    , m_timeout(timeout)
    , m_screen_box(feed, 0.2, 0.2, 0.6, 0.6)
    , m_dialog_box(feed, 0.50, 0.89, 0.40, 0.07)
    , m_start_time(std::chrono::system_clock::now())
{}

bool StartBattleDetector::has_timed_out() const{
    return std::chrono::system_clock::now() - m_start_time > m_timeout;
}
bool StartBattleDetector::detect(const QImage& screen){
    QImage image0 = extract_box(screen, m_screen_box);
    QImage image1 = extract_box(screen, m_dialog_box);

    ImageStats stats0 = pixel_stats(image0);
    ImageStats stats1 = pixel_stats(image1);
//    cout << "mean = " << stats.average << ", stddev = " << stats.stddev << endl;
//        return stats.stddev.sum() < 10;

    //  White screen.
    if (
        stats0.average.sum() > 600 && stats0.stddev.sum() < 10 &&
        stats1.average.sum() > 600 && stats1.stddev.sum() < 10
    ){
        return true;
    }

    //  Grey text box.
    bool dialog = stats0.stddev.sum() > 50 && is_dialog_grey(stats1);
    if (dialog){
//        cout << stats0.stddev.sum() << endl;
    }
    return dialog;
}
bool StartBattleDetector::wait(ProgramEnvironment& env, Logger& logger){
    InferenceThrottler throttler(m_timeout, std::chrono::milliseconds(50));
    while (true){
        env.check_stopping();

        QImage screen = m_feed.snapshot();
        if (detect(screen)){
            logger.log("StartBattleDetector: Detected start of battle!", "purple");
            return true;
        }

        if (throttler.end_iteration(env)){
            logger.log("StartBattleDetector: Timed out.", "red");
            return false;
        }
    }
}


}
}
}
