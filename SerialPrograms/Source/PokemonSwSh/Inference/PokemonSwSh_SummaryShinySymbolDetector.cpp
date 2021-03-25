/*  Summary Shiny Symbol
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Determine if a raid is full and ready to start early.
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/ColorClustering.h"
#include "PokemonSwSh_SummaryShinySymbolDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


SummaryShinySymbolDetector::SummaryShinySymbolDetector(VideoFeed& feed, Logger& logger)
    : m_feed(feed)
    , m_logger(logger)
    , m_state0_box(feed, 0.02, 0.84, 0.1, 0.1)
    , m_state1_box(feed, 0.02, 0.97, 0.5, 0.02)
    , m_symbol_box(feed, 0.08, 0.53, 0.02, 0.05)
{}

SummaryShinySymbolDetector::Detection SummaryShinySymbolDetector::detect_now(){
    QImage screen = m_feed.snapshot();

    {
        QImage state = extract_box(screen, m_state1_box);
        if (pixel_stddev(state).sum() > 10){
            return Detection::NO_DETECTION;
        }
        if (pixel_average(state).sum() > 100){
            return Detection::NO_DETECTION;
        }
    }
    {
        QImage state = extract_box(screen, m_state0_box);
        if (pixel_stddev(state).sum() > 10){
            return Detection::NO_DETECTION;
        }
        FloatPixel color = pixel_average_normalized(state);
        if (euclidean_distance(color, FloatPixel(0.70, 0.07, 0.23)) > 0.2){
            return Detection::NO_DETECTION;
        }
    }

    QImage symbol = extract_box(screen, m_symbol_box);
    if (cluster_fit_2(
        symbol,
        qRgb(255, 255, 255), 0.84,
        qRgb(156, 33, 80), 0.16,
        0.1
    )){
        return Detection::SHINY;
    }
    if (cluster_fit_2(
        symbol,
        qRgb(255, 255, 255), 0.92,
        qRgb(0, 0, 0), 0.08,
        0.1
    )){
//        screen.save("test.png");
        return Detection::NOT_SHINY;
    }

    return Detection::NO_DETECTION;
}
SummaryShinySymbolDetector::Detection SummaryShinySymbolDetector::wait_for_detection(
    ProgramEnvironment& env,
    std::chrono::seconds timeout
){
    auto start = std::chrono::system_clock::now();
    auto last = start;

    Detection last_detection = Detection::NO_DETECTION;
    size_t confirmations = 0;
    while (true){
        env.check_stopping();

        Detection detection = detect_now();
        if (detection == last_detection){
            confirmations++;
        }else{
            last_detection = detection;
            confirmations = 0;
        }
        if (last_detection != Detection::NO_DETECTION && confirmations >= 10){
            break;
        }

        auto now = std::chrono::system_clock::now();
        if (now - start > timeout){
            last_detection = Detection::NO_DETECTION;
            break;
        }
        auto time_since_last_frame = now - last;
        if (time_since_last_frame > std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - time_since_last_frame);
        }
        last = now;
    }

    switch (last_detection){
    case Detection::NO_DETECTION:
        m_logger.log("SummaryShinySymbolDetector: Nothing found after timeout.", "red");
        break;
    case Detection::NOT_SHINY:
        m_logger.log("SummaryShinySymbolDetector: Not shiny.", "purple");
        break;
    case Detection::SHINY:
        m_logger.log("SummaryShinySymbolDetector: Shiny!", "blue");
        break;
    }
    return last_detection;
}



}
}
}
