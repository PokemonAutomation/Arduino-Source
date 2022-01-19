/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP_ShinyTrigger.h"
#include "PokemonBDSP_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const std::chrono::milliseconds SHINY_ANIMATION_DELAY(3900);

const DetectionType WILD_POKEMON{
    {0.4, 0.02, 0.60, 0.93},
    PokemonSwSh::EncounterState::WILD_ANIMATION,
    std::chrono::milliseconds(3900),
    true,
};
const DetectionType YOUR_POKEMON{
    {0.0, 0.1, 0.8, 0.8},
    PokemonSwSh::EncounterState::YOUR_ANIMATION,
    std::chrono::milliseconds(3900),
    false,
};


class ShinyEncounterDetector{
public:
    ShinyEncounterDetector(
        Logger& logger, VideoOverlay& overlay,
        const DetectionType& type,
        double overall_threshold, double doubles_threshold
    );

    PokemonSwSh::EncounterState encounter_state() const{
        return m_dialog_tracker.encounter_state();
    }
    const QImage& best_screenshot() const{ return m_best; }

    DoublesShinyDetection results();

    void push(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()
    );
    void push_end(std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now());


private:
    Logger& m_logger;
    VideoOverlay& m_overlay;

    DetectionType m_type;

    InferenceBoxScope m_overall_box;
    InferenceBoxScope m_left_box;
    InferenceBoxScope m_right_box;
    VideoOverlaySet m_inference_boxes;

    std::chrono::milliseconds m_min_delay;
    std::chrono::milliseconds m_max_delay;

    double m_overall_threshold;
    double m_doubles_threshold;

    BattleDialogDetector m_dialog_detector;
    PokemonSwSh::EncounterDialogTracker m_dialog_tracker;

    std::deque<InferenceBoxScope> m_detection_overlays;

    bool m_dialog_trigger = false;
    ShinyImageAlpha m_alpha_both;
    ShinyImageAlpha m_alpha_left;
    ShinyImageAlpha m_alpha_right;

    QImage m_best;
    double m_best_type_alpha;
};


ShinyEncounterDetector::ShinyEncounterDetector(
    Logger& logger, VideoOverlay& overlay,
    const DetectionType& type,
    double overall_threshold, double doubles_threshold
)
    : m_logger(logger)
    , m_overlay(overlay)
    , m_type(type)
    , m_overall_box(overlay, type.box)
    , m_left_box(overlay, {0.40, 0.02, 0.20, 0.48})
    , m_right_box(overlay, {0.70, 0.02, 0.20, 0.48})
    , m_inference_boxes(overlay)
    , m_min_delay(SHINY_ANIMATION_DELAY - std::chrono::milliseconds(300))
    , m_max_delay(SHINY_ANIMATION_DELAY + std::chrono::milliseconds(500))
    , m_overall_threshold(overall_threshold)
    , m_doubles_threshold(doubles_threshold)
    , m_dialog_tracker(logger, m_dialog_detector)
    , m_best_type_alpha(0)
{
    m_dialog_tracker.make_overlays(m_inference_boxes);
}

void ShinyEncounterDetector::push(
    const QImage& screen,
    std::chrono::system_clock::time_point timestamp
){
    m_dialog_tracker.process_frame(screen, timestamp);
    std::chrono::milliseconds duration(0);
    switch (m_type.required_state){
    case PokemonSwSh::EncounterState::BEFORE_ANYTHING:
        break;
    case PokemonSwSh::EncounterState::WILD_ANIMATION:
        duration = m_dialog_tracker.wild_animation_duration();
        break;
    case PokemonSwSh::EncounterState::YOUR_ANIMATION:
        duration = m_dialog_tracker.your_animation_duration();
        break;
    case PokemonSwSh::EncounterState::POST_ENTRY:
        break;
    }
    m_dialog_trigger |= m_min_delay < duration && duration < m_max_delay;


    QImage overall_box = extract_box(screen, m_overall_box);
    QImage left_box = extract_box(screen, m_left_box);
    QImage right_box = extract_box(screen, m_right_box);

//    auto time0 = std::chrono::system_clock::now();
    ShinyImageDetection detections_overall;
    ShinyImageDetection detections_left;
    ShinyImageDetection detections_right;
    detections_overall.accumulate(overall_box, timestamp.time_since_epoch().count(), &m_logger);
    detections_left.accumulate(left_box, timestamp.time_since_epoch().count(), &m_logger);
    detections_right.accumulate(right_box, timestamp.time_since_epoch().count(), &m_logger);
//    auto time1 = std::chrono::system_clock::now();
//    cout << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

    ShinyImageAlpha alpha_overall = detections_overall.alpha();
    ShinyImageAlpha alpha_left = detections_left.alpha();
    ShinyImageAlpha alpha_right = detections_right.alpha();

    bool is_correct_state = m_dialog_tracker.encounter_state() == m_type.required_state;
    if (is_correct_state && !m_dialog_tracker.dialog_on()){
        m_alpha_both.max(alpha_overall);
        m_alpha_left.max(alpha_left);
        m_alpha_right.max(alpha_right);
    }

    if (alpha_overall.shiny > 0){
        QString str = "ShinyDetector: alpha = " +
            QString::number(alpha_overall.shiny) + " / "  +
            QString::number(m_alpha_both.shiny);

        if (alpha_overall.shiny >= m_overall_threshold){
            str += " (threshold exceeded)";
        }
        m_logger.log(str, COLOR_BLUE);

#if 1
        double type_alpha = alpha_overall.shiny;
        if (is_correct_state && type_alpha >= 2 && m_best_type_alpha == 0){
            m_best = screen;
            m_best_type_alpha = type_alpha;
            m_logger.log("ShinyDetector: New best screenshot: alpha = " + QString::number(type_alpha));
        }
#else
        double type_alpha = alpha_both.shiny;
        if (m_best_type_alpha < type_alpha){
            m_best = screen;
            m_best_type_alpha = type_alpha;
            m_logger.log("ShinyDetector: New best screenshot: alpha = " + QString::number(type_alpha));
        }
#endif
    }

    m_detection_overlays.clear();
    detections_overall.add_overlays(m_detection_overlays, m_overlay, screen, m_overall_box);
//    detections_left.add_overlays(m_detection_overlays, m_overlay, screen, m_doubles_left);
//    detections_right.add_overlays(m_detection_overlays, m_overlay, screen, m_doubles_right);
}
void ShinyEncounterDetector::push_end(std::chrono::system_clock::time_point timestamp){
    m_dialog_tracker.push_end(timestamp);
}

DoublesShinyDetection ShinyEncounterDetector::results(){
    double alpha = m_alpha_both.shiny;
    if (m_dialog_trigger){
        alpha += 3.5;
    }
    m_logger.log(
        "ShinyDetector: Overall Alpha = " + QString::number(alpha) +
        ", Left Alpha = " + QString::number(m_alpha_left.shiny) +
        ", Right Alpha = " + QString::number(m_alpha_right.shiny),
        COLOR_PURPLE
    );

    DoublesShinyDetection result;
    result.shiny_type = ShinyType::UNKNOWN;
    result.best_screenshot = std::move(m_best);
    result.left_is_shiny = false;
    result.right_is_shiny = false;


    if (alpha < m_overall_threshold){
        m_logger.log("ShinyDetector: Not Shiny.", COLOR_PURPLE);
        result.shiny_type = ShinyType::NOT_SHINY;
        return result;
    }

    m_logger.log("ShinyDetector: Detected Shiny!", COLOR_BLUE);
    result.shiny_type = ShinyType::UNKNOWN_SHINY;
    result.left_is_shiny = m_alpha_left.shiny >= m_doubles_threshold;
    result.right_is_shiny = m_alpha_right.shiny >= m_doubles_threshold;
//    result.left_is_shiny = false;
    return result;
}





DoublesShinyDetection detect_shiny_battle(
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    const DetectionType& type,
    std::chrono::seconds timeout,
    double overall_threshold, double doubles_threshold
){
    StatAccumulatorI32 capture_stats;
    StatAccumulatorI32 menu_stats;
    StatAccumulatorI32 inference_stats;
    StatAccumulatorI32 throttle_stats;

    BattleMenuWatcher menu(type.full_battle_menu ? BattleType::WILD : BattleType::STARTER);
    VideoOverlaySet overlay_boxes(overlay);
    menu.make_overlays(overlay_boxes);
    ShinyEncounterDetector detector(
        logger, overlay,
        type,
        overall_threshold, doubles_threshold
    );

    bool no_detection = false;

    InferenceThrottler throttler(timeout, std::chrono::milliseconds(50));
    while (true){
        env.check_stopping();

        auto time0 = std::chrono::system_clock::now();
        QImage screen = feed.snapshot();
//        if (screen.isNull()){
//
//        }
        auto time1 = std::chrono::system_clock::now();
        capture_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
        auto timestamp = time1;

#if 0
//        cout << (int)detector.encounter_state() << endl;
        if (detector.encounter_state() == PokemonSwSh::EncounterState::YOUR_ANIMATION){
            env.log("ShinyDetector: End of wild entry animation.", COLOR_PURPLE);
            break;
        }
#endif

        if (menu.detect(screen)){
            env.log("ShinyDetector: Battle menu found!", COLOR_PURPLE);
            detector.push_end();
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
            dump_image(logger, env.program_info(), "BattleMenu", screen);
            break;
        }
        auto time4 = std::chrono::system_clock::now();
        throttle_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time4 - time3).count();
    }

    env.log("Diagnostics: Screenshot: " + capture_stats.dump(), COLOR_MAGENTA);
    env.log("Diagnostics: Menu Detection: " + menu_stats.dump(), COLOR_MAGENTA);
    env.log("Diagnostics: Inference: " + inference_stats.dump(), COLOR_MAGENTA);
    env.log("Diagnostics: Throttle: " + throttle_stats.dump(), COLOR_MAGENTA);

    if (no_detection){
        env.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
        return DoublesShinyDetection();
    }

    return detector.results();
}









}
}
}
