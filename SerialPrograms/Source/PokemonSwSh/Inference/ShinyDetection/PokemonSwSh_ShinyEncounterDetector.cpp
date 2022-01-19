/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"
#include "PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const ShinyDetectionBattle SHINY_BATTLE_REGULAR {false, {0.5, 0.05, 0.5, 0.70}, std::chrono::milliseconds(2300)};
const ShinyDetectionBattle SHINY_BATTLE_RAID    {true,  {0.3, 0.01, 0.7, 0.75}, std::chrono::milliseconds(3900)};



class ShinyEncounterDetector{
public:
    ShinyEncounterDetector(
        Logger& logger, VideoOverlay& overlay,
        const ShinyDetectionBattle& battle_settings,
        double detection_threshold
    );

    ShinyType shiny_type() const;
    EncounterState encounter_state() const{ return m_dialog_tracker.encounter_state(); }

    const QImage& best_screenshot() const{ return m_best; }

    ShinyDetectionResult results(){
        ShinyDetectionResult result;
        result.shiny_type = shiny_type();
        result.best_screenshot = std::move(m_best);
        return result;
    }

    void push(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()
    );


private:
    Logger& m_logger;
    VideoOverlay& m_overlay;

//    InferenceBoxScope m_dialog_box;
    InferenceBoxScope m_shiny_box;
    VideoOverlaySet m_inference_boxes;

    std::chrono::milliseconds m_min_delay;
    std::chrono::milliseconds m_max_delay;

    double m_detection_threshold;

    BattleDialogDetector m_dialog_detector;
    EncounterDialogTracker m_dialog_tracker;

    std::deque<InferenceBoxScope> m_detection_overlays;

    bool m_dialog_trigger = false;
    ShinyImageAlpha m_image_alpha;

    QImage m_best;
    double m_best_type_alpha;
};




ShinyEncounterDetector::ShinyEncounterDetector(
    Logger& logger, VideoOverlay& overlay,
    const ShinyDetectionBattle& battle_settings,
    double detection_threshold
)
    : m_logger(logger)
    , m_overlay(overlay)
//    , m_dialog_box(overlay, battle_settings.detection_box)
    , m_shiny_box(overlay, battle_settings.detection_box)
    , m_inference_boxes(overlay)
    , m_min_delay(battle_settings.dialog_delay_when_shiny - std::chrono::milliseconds(300))
    , m_max_delay(battle_settings.dialog_delay_when_shiny + std::chrono::milliseconds(500))
    , m_detection_threshold(detection_threshold)
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
    auto wild_animation_duration = m_dialog_tracker.wild_animation_duration();
    m_dialog_trigger |= m_min_delay < wild_animation_duration && wild_animation_duration < m_max_delay;


    QImage shiny_box = extract_box(screen, m_shiny_box);

//    auto time0 = std::chrono::system_clock::now();
    ShinyImageDetection signatures;
    signatures.accumulate(shiny_box, timestamp.time_since_epoch().count(), &m_logger);
//    auto time1 = std::chrono::system_clock::now();
//    cout << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

    ShinyImageAlpha frame_alpha = signatures.alpha();

    if (m_dialog_tracker.encounter_state() == EncounterState::WILD_ANIMATION &&
        !m_dialog_tracker.dialog_on()
    ){
        m_image_alpha.max(frame_alpha);
    }

    if (frame_alpha.shiny > 0){
        QString str = "ShinyDetector: alpha = " +
            QString::number(frame_alpha.shiny) + " / "  +
            QString::number(m_image_alpha.shiny);

        if (frame_alpha.shiny >= m_detection_threshold){
            str += " (threshold exceeded)";
        }
        m_logger.log(str, COLOR_BLUE);

        double type_alpha = frame_alpha.star + frame_alpha.square;
        if (m_best_type_alpha < type_alpha){
            m_best = screen;
            m_best_type_alpha = type_alpha;
            m_logger.log("ShinyDetector: New best screenshot: alpha = " + QString::number(type_alpha));
        }
    }

    m_detection_overlays.clear();
    for (const auto& item : signatures.balls){
        ImageFloatBox box = translate_to_parent(screen, m_shiny_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.stars){
        ImageFloatBox box = translate_to_parent(screen, m_shiny_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.squares){
        ImageFloatBox box = translate_to_parent(screen, m_shiny_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
    for (const auto& item : signatures.lines){
        ImageFloatBox box = translate_to_parent(screen, m_shiny_box, item);
        m_detection_overlays.emplace_back(m_overlay, box, COLOR_GREEN);
    }
}

ShinyType ShinyEncounterDetector::shiny_type() const{
    double alpha = m_image_alpha.shiny;
    if (m_dialog_trigger){
        alpha += 1.4;
    }
    m_logger.log(
        "ShinyDetector: Overall Alpha = " + QString::number(alpha) +
        ", Star Alpha = " + QString::number(m_image_alpha.star) +
        ", Square Alpha = " + QString::number(m_image_alpha.square),
        COLOR_PURPLE
    );

    if (alpha < m_detection_threshold){
        m_logger.log("ShinyDetector: Not Shiny.", COLOR_PURPLE);
        return ShinyType::NOT_SHINY;
    }
    if (m_image_alpha.star > 0 && m_image_alpha.star > m_image_alpha.square * 2){
        m_logger.log("ShinyDetector: Detected Star Shiny!", COLOR_BLUE);
        return ShinyType::STAR_SHINY;
    }
    if (m_image_alpha.square > 0 && m_image_alpha.square > m_image_alpha.star * 2){
        m_logger.log("ShinyDetector: Detected Square Shiny!", COLOR_BLUE);
        return ShinyType::SQUARE_SHINY;
    }

    m_logger.log("ShinyDetector: Detected Shiny! But ambiguous shiny type.", COLOR_BLUE);
    return ShinyType::UNKNOWN_SHINY;
}



ShinyDetectionResult detect_shiny_battle(
    ProgramEnvironment& env,
    Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold
){
    if (GameSettings::instance().USE_NEW_SHINY_DETECTOR){
        ShinyEncounterTracker tracker(logger, overlay, battle_settings);
        int result = wait_until(
            env, logger, feed, overlay, timeout,
            { &tracker }
        );
        if (result < 0){
            env.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
            return ShinyDetectionResult{ShinyType::UNKNOWN, QImage()};
        }
        ShinyType shiny_type = determine_shiny_status(
            logger,
            battle_settings,
            tracker.dialog_timer(),
            tracker.sparkles_wild()
        );
        return ShinyDetectionResult{shiny_type, tracker.sparkles_wild().best_image()};
    }

    StatAccumulatorI32 capture_stats;
    StatAccumulatorI32 menu_stats;
    StatAccumulatorI32 inference_stats;
    StatAccumulatorI32 throttle_stats;

    StandardBattleMenuWatcher menu(battle_settings.den);
    VideoOverlaySet overlay_boxes(overlay);
    menu.make_overlays(overlay_boxes);
    ShinyEncounterDetector detector(
        logger,
        overlay,
        battle_settings,
        detection_threshold
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

        if (menu.detect(screen)){
            env.log("ShinyDetector: Battle menu found!", COLOR_PURPLE);
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
        return ShinyDetectionResult{ShinyType::UNKNOWN, QImage()};
    }

    return detector.results();
}





ShinyEncounterTracker::ShinyEncounterTracker(
    Logger& logger, VideoOverlay& overlay,
    const ShinyDetectionBattle& battle_settings
)
    : VisualInferenceCallback("ShinyEncounterTracker")
    , m_battle_settings(battle_settings)
    , m_logger(logger)
//    , m_overlay(overlay)
    , m_battle_menu(battle_settings.den)
    , m_dialog_tracker(logger, m_dialog_detector)
    , m_sparkle_tracker(logger, overlay, m_sparkles, battle_settings.detection_box)
{}
void ShinyEncounterTracker::make_overlays(VideoOverlaySet& items) const{
    m_battle_menu.make_overlays(items);
    m_dialog_tracker.make_overlays(items);
    m_sparkle_tracker.make_overlays(items);
}
bool ShinyEncounterTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    bool battle_menu = m_battle_menu.process_frame(frame, timestamp);
    if (battle_menu){
        m_dialog_tracker.push_end(timestamp);
        return true;
    }

    m_dialog_tracker.process_frame(frame, timestamp);
    m_sparkle_tracker.process_frame(frame, timestamp);

    switch (m_dialog_tracker.encounter_state()){
    case EncounterState::BEFORE_ANYTHING:
        break;
    case EncounterState::WILD_ANIMATION:
        m_best_wild.add_frame(frame, m_sparkles);
        break;
    case EncounterState::YOUR_ANIMATION:
        break;
    case EncounterState::POST_ENTRY:
        break;
    }

    return false;
}

ShinyType determine_shiny_status(
    Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    const EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles,
    double detection_threshold
){
    double alpha = sparkles.best_overall();

    std::chrono::milliseconds dialog_duration = dialog_tracker.wild_animation_duration();
    std::chrono::milliseconds min_delay = battle_settings.dialog_delay_when_shiny - std::chrono::milliseconds(300);
    std::chrono::milliseconds max_delay = battle_settings.dialog_delay_when_shiny + std::chrono::milliseconds(500);
    if (min_delay <= dialog_duration && dialog_duration <= max_delay){
        alpha += 1.2;
    }

    double best_star = sparkles.best_star();
    double best_square = sparkles.best_square();

    logger.log(
        "ShinyDetector: Overall Alpha = " + QString::number(alpha) +
        ", Star Alpha = " + QString::number(best_star) +
        ", Square Alpha = " + QString::number(best_square),
        COLOR_PURPLE
    );

    if (alpha < detection_threshold){
        logger.log("ShinyDetector: Not Shiny.", COLOR_PURPLE);
        return ShinyType::NOT_SHINY;
    }
    if (best_star > 0 && best_star > best_square){
        logger.log("ShinyDetector: Detected Star Shiny!", COLOR_BLUE);
        return ShinyType::STAR_SHINY;
    }
    if (best_square > 0 && best_square > best_star * 2){
        logger.log("ShinyDetector: Detected Square Shiny!", COLOR_BLUE);
        return ShinyType::SQUARE_SHINY;
    }

    logger.log("ShinyDetector: Detected Shiny! But ambiguous shiny type.", COLOR_BLUE);
    return ShinyType::UNKNOWN_SHINY;
}






}
}
}
