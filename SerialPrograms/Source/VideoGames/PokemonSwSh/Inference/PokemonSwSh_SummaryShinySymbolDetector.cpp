/*  Summary Shiny Symbol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ColorClustering.h"
#include "CommonTools/InferenceThrottler.h"
#include "PokemonSwSh_SummaryShinySymbolDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


SummaryShinySymbolDetector::SummaryShinySymbolDetector(Logger& logger, VideoOverlay& overlay)
    : m_logger(logger)
    , m_state0_box(overlay, {0.02, 0.84, 0.1, 0.1})
    , m_state1_box(overlay, {0.02, 0.97, 0.5, 0.02})
    , m_symbol_box(overlay, {0.08, 0.53, 0.02, 0.05})
{}

SummaryShinySymbolDetector::Detection SummaryShinySymbolDetector::detect(const ImageViewRGB32& screen){
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_state1_box));
        if (!is_black(stats)){
            return Detection::NO_DETECTION;
        }
    }
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_state0_box));
        if (!is_solid(stats, {0.70, 0.07, 0.23}, 0.2, 10)){
            return Detection::NO_DETECTION;
        }
    }

    ImageViewRGB32 symbol = extract_box_reference(screen, m_symbol_box);
    if (cluster_fit_2(
        symbol,
        Color(255, 255, 255), 0.84,
        Color(156, 33, 80), 0.16,
        0.1
    )){
//        symbol.save("test.png");
        return Detection::SHINY;
    }
    if (cluster_fit_2(
        symbol,
        Color(255, 255, 255), 0.92,
        Color(0, 0, 0), 0.08,
        0.1
    )){
        return Detection::NOT_SHINY;
    }

    return Detection::NO_DETECTION;
}
SummaryShinySymbolDetector::Detection SummaryShinySymbolDetector::wait_for_detection(
    CancellableScope& scope, VideoFeed& feed,
    std::chrono::seconds timeout
){
    Detection last_detection = Detection::NO_DETECTION;
    size_t confirmations = 0;

    InferenceThrottler throttler(timeout);
    while (true){
        scope.throw_if_cancelled();

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

        if (throttler.end_iteration(scope)){
            last_detection = Detection::NO_DETECTION;
            break;
        }
    }

    switch (last_detection){
    case Detection::NO_DETECTION:
        m_logger.log("SummaryShinySymbolDetector: Nothing found after timeout.", COLOR_RED);
        break;
    case Detection::NOT_SHINY:
        m_logger.log("SummaryShinySymbolDetector: Not shiny.", COLOR_PURPLE);
        break;
    case Detection::SHINY:
        m_logger.log("SummaryShinySymbolDetector: Shiny!", COLOR_BLUE);
        break;
    }
    return last_detection;
}



}
}
}
