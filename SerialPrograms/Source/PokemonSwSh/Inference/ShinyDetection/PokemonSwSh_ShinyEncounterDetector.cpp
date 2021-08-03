/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyDialogTracker.h"
#include "PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const ShinyDetectionBattle SHINY_BATTLE_REGULAR {{0.5, 0.05, 0.5, 0.70}, std::chrono::milliseconds(2300)};
const ShinyDetectionBattle SHINY_BATTLE_RAID    {{0.3, 0.01, 0.7, 0.75}, std::chrono::milliseconds(3900)};



class ShinyEncounterDetector{
public:
    ShinyEncounterDetector(
        VideoFeed& feed, Logger& logger,
        const ShinyDetectionBattle& battle_settings,
        double detection_threshold
    );

    ShinyType results() const;

    void push(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()
    );


private:
    VideoFeed& m_feed;
    Logger& m_logger;

//    InferenceBoxScope m_dialog_box;
    InferenceBoxScope m_shiny_box;

    std::chrono::milliseconds m_min_delay;
    std::chrono::milliseconds m_max_delay;

    double m_detection_threshold;
    StandardBattleMenuDetector m_menu;

    ShinyDialogTracker m_dialog_tracker;

    std::deque<InferenceBoxScope> m_detection_overlays;

    bool m_dialog_trigger = false;
    ShinyImageAlpha m_image_alpha;

};




ShinyEncounterDetector::ShinyEncounterDetector(
    VideoFeed& feed, Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    double detection_threshold
)
    : m_feed(feed)
    , m_logger(logger)
//    , m_dialog_box(feed, 0.50, 0.89, 0.40, 0.07)
    , m_shiny_box(feed, battle_settings.detection_box)
    , m_min_delay(battle_settings.dialog_delay_when_shiny - std::chrono::milliseconds(500))
    , m_max_delay(battle_settings.dialog_delay_when_shiny + std::chrono::milliseconds(500))
    , m_detection_threshold(detection_threshold)
    , m_menu(feed)
    , m_dialog_tracker(feed, logger)
{}


void ShinyEncounterDetector::push(
    const QImage& screen,
    std::chrono::system_clock::time_point timestamp
){
    m_dialog_tracker.push_frame(screen, timestamp);
    auto wild_animation_duration = m_dialog_tracker.wild_animation_duration();
    m_dialog_trigger |= m_min_delay < wild_animation_duration && wild_animation_duration < m_max_delay;




    QImage shiny_box = extract_box(screen, m_shiny_box);

//    auto time0 = std::chrono::system_clock::now();
    ShinyImageDetection signatures;
    signatures.accumulate(shiny_box, timestamp.time_since_epoch().count(), &m_logger);
//    auto time1 = std::chrono::system_clock::now();
//    cout << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

    ShinyImageAlpha frame_alpha = signatures.alpha();

    if (m_dialog_tracker.encounter_state() == EncounterState::WILD_ANIMATION){
        m_image_alpha.max(frame_alpha);
    }

    if (frame_alpha.shiny > 0){
        if (frame_alpha.shiny >= m_detection_threshold){
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny) + " (threshold exceeded)",
                "blue"
            );
        }else{
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny),
                "blue"
            );
        }
    }

    m_detection_overlays.clear();
    for (const auto& item : signatures.balls){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.stars){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.squares){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.lines){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
}

ShinyType ShinyEncounterDetector::results() const{
    double alpha = m_image_alpha.shiny;
    if (m_dialog_trigger){
        alpha += 1.4;
    }
    m_logger.log(
        "ShinyDetector: Overall Alpha = " + QString::number(alpha) +
        ", Star Alpha = " + QString::number(m_image_alpha.star) +
        ", Square Alpha = " + QString::number(m_image_alpha.square),
        "purple"
    );

    if (alpha < m_detection_threshold){
        m_logger.log("ShinyDetector: Not Shiny.", "purple");
        return ShinyType::NOT_SHINY;
    }
    if (m_image_alpha.star > 0 && m_image_alpha.star > m_image_alpha.square * 2){
        m_logger.log("ShinyDetector: Detected Star Shiny!", "blue");
        return ShinyType::STAR_SHINY;
    }
    if (m_image_alpha.square > 0 && m_image_alpha.square > m_image_alpha.star * 2){
        m_logger.log("ShinyDetector: Detected Square Shiny!", "blue");
        return ShinyType::SQUARE_SHINY;
    }

    m_logger.log("ShinyDetector: Detected Shiny! But ambiguous shiny type.", "blue");
    return ShinyType::UNKNOWN_SHINY;
}




ShinyType detect_shiny_battle(
    ProgramEnvironment& env, VideoFeed& feed,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold
){
    StatAccumulatorI32 capture_stats;
    StatAccumulatorI32 menu_stats;
    StatAccumulatorI32 inference_stats;
    StatAccumulatorI32 throttle_stats;

    StandardBattleMenuDetector menu(feed);
    ShinyEncounterDetector detector(feed, env.logger(), battle_settings, detection_threshold);

    bool no_detection = false;

    InferenceThrottler throttler(timeout, std::chrono::milliseconds(50));
    while (true){
        env.check_stopping();

        auto time0 = std::chrono::system_clock::now();
        QImage screen = feed.snapshot();
        if (screen.isNull()){

        }
        auto time1 = std::chrono::system_clock::now();
        capture_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
        auto timestamp = time1;

        if (menu.detect(screen)){
            env.log("ShinyDetector: Battle menu found!", "purple");
            break;
        }
        auto time2 = std::chrono::system_clock::now();
        menu_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();

        detector.push(screen, timestamp);
        auto time3 = std::chrono::system_clock::now();
        inference_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time3 - time2).count();

//        if (time3 - time2 > std::chrono::milliseconds(50)){
//            screen.save("slow-inference.png");
//        }

        if (throttler.end_iteration(env)){
            no_detection = true;
            break;
        }
        auto time4 = std::chrono::system_clock::now();
        throttle_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time4 - time3).count();
    }

    env.log("Diagnostics: Screenshot: " + capture_stats.dump(), Qt::magenta);
    env.log("Diagnostics: Menu Detection: " + menu_stats.dump(), Qt::magenta);
    env.log("Diagnostics: Inference: " + inference_stats.dump(), Qt::magenta);
    env.log("Diagnostics: Throttle: " + throttle_stats.dump(), Qt::magenta);

    if (no_detection){
        env.log("ShinyDetector: Battle menu not found after timeout.", "red");
        return ShinyType::UNKNOWN;
    }

    return detector.results();
}



}
}
}
