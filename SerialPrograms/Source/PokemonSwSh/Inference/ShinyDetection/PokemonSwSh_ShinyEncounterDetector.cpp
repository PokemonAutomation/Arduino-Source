/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "CommonFramework/Inference/FloatStatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

const InferenceBox ShinyEncounterDetector::REGULAR_BATTLE(0.5, 0.05, 0.5, 0.70);
const InferenceBox ShinyEncounterDetector::RAID_BATTLE(0.3, 0.01, 0.7, 0.75);


ShinyEncounterDetector::ShinyEncounterDetector(
    VideoFeed& feed, Logger& logger,
    const InferenceBox& box,
    std::chrono::seconds timeout,
    double detection_threshold
)
    : m_feed(feed)
    , m_logger(logger)
    , m_box(feed, box)
    , m_timeout(timeout)
    , m_detection_threshold(detection_threshold)
    , m_menu(feed)
{}

ShinyEncounterDetector::Detection ShinyEncounterDetector::detect(ProgramEnvironment& env){
    TimeWindowStatTracker<FloatStatAccumulator> tracker(std::chrono::milliseconds(4000));

    std::deque<InferenceBoxScope> detection_overlays;

    bool shiny = false;
    double star_alpha = 0;
    double square_alpha = 0;

    auto start = std::chrono::system_clock::now();
    auto last = start;
    for (uint64_t count = 0;; count++){
        env.check_stopping();

        QImage image = m_feed.snapshot();
        if (m_menu.detect(image)){
            m_logger.log("ShinyDetector: Battle menu found!", "purple");
            break;
        }

        QImage inference_image = extract_box(image, m_box);

        ShinyImageDetection signatures;
        signatures.accumulate(inference_image, count, &m_logger);

        star_alpha += 3 * signatures.stars.size();
        if (signatures.squares.size() > 1){
            square_alpha += signatures.squares.size() - 1;
        }
        square_alpha += 5 * signatures.lines.size();

        double signature_alpha = signatures.alpha();
        tracker.push(signature_alpha, last);
        FloatStatAccumulator previous_2_seconds = tracker.accumulate_start_to_point(std::chrono::milliseconds(2000));
        FloatStatAccumulator last_2_seconds = tracker.accumulate_last(std::chrono::milliseconds(2000));
        double mean0 = previous_2_seconds.mean();
        double alpha = last_2_seconds.diff_metric(mean0);

        if (signature_alpha > 1.0){
            if (alpha > m_detection_threshold){
                shiny = true;
                m_logger.log(
                    "ShinyDetector: alpha = " + QString::number(alpha) + " (threshold exceeded)",
                    "blue"
                );
            }else{
                m_logger.log(
                    "ShinyDetector: alpha = " + QString::number(alpha),
                    "blue"
                );
            }
        }

        detection_overlays.clear();
        for (const auto& item : signatures.balls){
            InferenceBox box = translate_to_parent(image, m_box, item);
            box.color = Qt::green;
            detection_overlays.emplace_back(m_feed, box);
        }
        for (const auto& item : signatures.stars){
            InferenceBox box = translate_to_parent(image, m_box, item);
            box.color = Qt::green;
            detection_overlays.emplace_back(m_feed, box);
        }
        for (const auto& item : signatures.squares){
            InferenceBox box = translate_to_parent(image, m_box, item);
            box.color = Qt::green;
            detection_overlays.emplace_back(m_feed, box);
        }
        for (const auto& item : signatures.lines){
            InferenceBox box = translate_to_parent(image, m_box, item);
            box.color = Qt::green;
            detection_overlays.emplace_back(m_feed, box);
        }

        if (!signatures.lines.empty()){
            image.save("DetectionLine.png");
        }

        auto now = std::chrono::system_clock::now();
        if (now - start > m_timeout){
            m_logger.log("ShinyDetector: Battle menu not found after timeout.", "red");
            return Detection::NO_BATTLE_MENU;
        }
        auto time_since_last_frame = now - last;
        if (time_since_last_frame > std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - time_since_last_frame);
        }
        last = now;
    }
    detection_overlays.clear();

    if (!shiny){
        m_logger.log("ShinyDetector: Not Shiny.", "purple");
        return Detection::NOT_SHINY;
    }

    if (star_alpha >= square_alpha){
        m_logger.log("ShinyDetector: Detected Star Shiny!", "blue");
        return Detection::STAR_SHINY;
    }else{
        m_logger.log("ShinyDetector: Detected Square Shiny!", "blue");
        return Detection::SQUARE_SHINY;
    }
}



}
}
}
