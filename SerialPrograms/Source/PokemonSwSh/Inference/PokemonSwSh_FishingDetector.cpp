/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh_FishingDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


FishingDetector::FishingDetector(
    VideoFeed& feed
)
    : m_feed(feed)
    , m_hook_box(feed, 0.4, 0.15, 0.2, 0.4)
    , m_miss_box(feed, 0.3, 0.9, 0.4, 0.05)
    , m_battle_menu(feed)
{}
FishingDetector::Detection FishingDetector::detect_now(){
    QImage screen = m_feed.snapshot();

    if (m_battle_menu.detect(screen)){
        return Detection::BATTLE_MENU;
    }

    QImage hook_image = extract_box(screen, m_hook_box);
    {
        QImage image = extract_box(screen, m_miss_box);
        ImageStats stats = pixel_stats(image);
        if (stats.stddev.sum() < 10 && stats.average.sum() > 500 && pixel_stddev(hook_image).sum() > 50){
            return Detection::MISSED;
        }
    }

    std::vector<PixelBox> exclamation_marks;
    find_marks(hook_image, &exclamation_marks, nullptr);

    return exclamation_marks.empty()
        ? Detection::NO_DETECTION
        : Detection::HOOKED;
}
FishingDetector::Detection FishingDetector::wait_for_detection(
    ProgramEnvironment& env, Logger& logger,
    std::chrono::seconds timeout
){
    InferenceThrottler throttler(timeout);
    while (true){
        env.check_stopping();

        Detection detection = detect_now();
        switch (detection){
        case Detection::NO_DETECTION:
            break;
        case Detection::HOOKED:
            logger.log("FishEncounterDetector: Detected hook!", "purple");
            return detection;
        case Detection::MISSED:
            logger.log("FishEncounterDetector: Missed a hook.", "red");
            return detection;
        case Detection::BATTLE_MENU:
            logger.log("FishEncounterDetector: Expected battle menu.", "red");
            return detection;
        }

        if (throttler.end_iteration(env)){
        logger.log("FishEncounterDetector: Timed out.", "red");
            return Detection::NO_DETECTION;
        }
    }
}



}
}
}

