/*  Summary Shiny Symbol
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Determine if a raid is full and ready to start early.
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh_SummaryShinySymbolDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


SummaryShinySymbolDetector::SummaryShinySymbolDetector(Logger& logger, VideoOverlay& overlay)
    : m_logger(logger)
    , m_state0_box(overlay, 0.02, 0.84, 0.1, 0.1)
    , m_state1_box(overlay, 0.02, 0.97, 0.5, 0.02)
    , m_symbol_box(overlay, 0.08, 0.53, 0.02, 0.05)
{}

SummaryShinySymbolDetector::Detection SummaryShinySymbolDetector::detect(const QImage& screen){
    {
        QImage state = extract_box(screen, m_state1_box);
        ImageStats stats = image_stats(state);
        if (!is_black(stats)){
            return Detection::NO_DETECTION;
        }
    }
    {
        QImage state = extract_box(screen, m_state0_box);
        ImageStats stats = image_stats(state);
        if (!is_solid(stats, {0.70, 0.07, 0.23}, 0.2, 10)){
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
    VideoFeed& feed,
    std::chrono::seconds timeout
){
    Detection last_detection = Detection::NO_DETECTION;
    size_t confirmations = 0;

    InferenceThrottler throttler(timeout);
    while (true){
        env.check_stopping();

        Detection detection = detect(feed.snapshot());
        if (detection == last_detection){
            confirmations++;
        }else{
            last_detection = detection;
            confirmations = 0;
        }
        if (last_detection != Detection::NO_DETECTION && confirmations >= 10){
            break;
        }

        if (throttler.end_iteration(env)){
            last_detection = Detection::NO_DETECTION;
            break;
        }
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
