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


FishingDetector::FishingDetector(VideoOverlay& overlay)
    : m_overlay(overlay)
    , m_hook_box(overlay, 0.1, 0.15, 0.8, 0.4)
    , m_miss_box(overlay, 0.3, 0.9, 0.4, 0.05)
    , m_battle_menu(overlay, false)
{}
FishingDetector::Detection FishingDetector::detect_now(const QImage& screen){
    if (m_battle_menu.detect(screen)){
        return Detection::BATTLE_MENU;
    }

    QImage hook_image = extract_box(screen, m_hook_box);
    {
        QImage image = extract_box(screen, m_miss_box);
        ImageStats stats = image_stats(image);
        if (stats.stddev.sum() < 10 && stats.average.sum() > 500 && pixel_stddev(hook_image).sum() > 50){
            return Detection::MISSED;
        }
    }

    std::vector<ImagePixelBox> exclamation_marks;
    find_marks(hook_image, &exclamation_marks, nullptr);
    for (const ImagePixelBox& mark : exclamation_marks){
        ImageFloatBox box = translate_to_parent(screen, m_hook_box, mark);
        box.x -= box.width * 1.5;
        box.width *= 4;
        box.height *= 1.5;
        m_marks.emplace_back(m_overlay, box, Qt::yellow);
    }

    return exclamation_marks.empty()
        ? Detection::NO_DETECTION
        : Detection::HOOKED;
}
FishingDetector::Detection FishingDetector::wait_for_detection(
    ProgramEnvironment& env,
    VideoFeed& feed,
    std::chrono::seconds timeout
){
    InferenceThrottler throttler(timeout);
    while (true){
        env.check_stopping();

        Detection detection = detect_now(feed.snapshot());
        switch (detection){
        case Detection::NO_DETECTION:
            break;
        case Detection::HOOKED:
            env.log("FishEncounterDetector: Detected hook!", "purple");
            return detection;
        case Detection::MISSED:
            env.log("FishEncounterDetector: Missed a hook.", "red");
            return detection;
        case Detection::BATTLE_MENU:
            env.log("FishEncounterDetector: Expected battle menu.", "red");
            return detection;
        }

        if (throttler.end_iteration(env)){
            env.log("FishEncounterDetector: Timed out.", "red");
            return Detection::NO_DETECTION;
        }
    }
}



}
}
}

