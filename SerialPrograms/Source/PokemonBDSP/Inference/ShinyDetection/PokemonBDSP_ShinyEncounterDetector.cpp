/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

//#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
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



ShinyEncounterTracker::ShinyEncounterTracker(
    LoggerQt& logger, VideoOverlay& overlay,
    BattleType battle_type
)
    : VisualInferenceCallback("ShinyEncounterTracker")
    , m_logger(logger)
//    , m_overlay(overlay)
    , m_battle_menu(battle_type)
    , m_dialog_tracker(logger, m_dialog_detector)
    , m_box_wild_left(0.40, 0.02, 0.20, 0.48)
    , m_box_wild_right(0.70, 0.02, 0.20, 0.48)
    , m_sparkle_tracker_wild(logger, overlay, m_sparkles_wild, {0.4, 0.02, 0.60, 0.93})
    , m_sparkle_tracker_own(logger, overlay, m_sparkles_own, {0.0, 0.1, 0.8, 0.8})
{}
void ShinyEncounterTracker::make_overlays(VideoOverlaySet& items) const{
    m_battle_menu.make_overlays(items);
    m_dialog_tracker.make_overlays(items);
    items.add(COLOR_RED, m_box_wild_left);
    items.add(COLOR_RED, m_box_wild_right);
    m_sparkle_tracker_wild.make_overlays(items);
    m_sparkle_tracker_own.make_overlays(items);
}
bool ShinyEncounterTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    using PokemonSwSh::EncounterState;

    if (frame.isNull()){
        return false;
    }

    bool battle_menu = m_battle_menu.process_frame(frame, timestamp);
    if (battle_menu){
        m_dialog_tracker.push_end(timestamp);
        return true;
    }

    m_dialog_tracker.process_frame(frame, timestamp);

    switch (m_dialog_tracker.encounter_state()){
    case EncounterState::BEFORE_ANYTHING:
        break;
    case EncounterState::WILD_ANIMATION:{
        m_sparkle_tracker_wild.process_frame(frame, timestamp);
        m_sparkle_tracker_own.clear_boxes();
        m_best_wild_overall.add_frame(frame, m_sparkles_wild);

        ImagePixelBox box_overall = floatbox_to_pixelbox(frame.width(), frame.height(), {0.4, 0.02, 0.60, 0.93});
        ImagePixelBox box_left = floatbox_to_pixelbox(frame.width(), frame.height(), m_box_wild_left);
        ImagePixelBox box_right = floatbox_to_pixelbox(frame.width(), frame.height(), m_box_wild_right);
        box_left.min_x -= box_overall.min_x;
        box_left.min_y -= box_overall.min_y;
        box_left.max_x -= box_overall.min_x;
        box_left.max_y -= box_overall.min_y;
        box_right.min_x -= box_overall.min_x;
        box_right.min_y -= box_overall.min_y;
        box_right.max_x -= box_overall.min_x;
        box_right.max_y -= box_overall.min_y;

        m_best_wild_left.add_frame(QImage(), m_sparkles_wild.extract_subbox(box_left));
        m_best_wild_right.add_frame(QImage(), m_sparkles_wild.extract_subbox(box_right));
        break;
    }
    case EncounterState::YOUR_ANIMATION:
        m_sparkle_tracker_wild.clear_boxes();
        m_sparkle_tracker_own.process_frame(frame, timestamp);
        m_best_own.add_frame(frame, m_sparkles_own);
        break;
    case EncounterState::POST_ENTRY:
        break;
    }

    return false;
}

void determine_shiny_status(
    LoggerQt& logger,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    const PokemonSwSh::EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles_wild_overall,
    const ShinySparkleAggregator& sparkles_wild_left,
    const ShinySparkleAggregator& sparkles_wild_right,
    const ShinySparkleAggregator& sparkles_own,
    double overall_threshold,
    double doubles_threshold
){
    double alpha_wild_overall = sparkles_wild_overall.best_overall();
    double alpha_wild_left = sparkles_wild_left.best_overall();
    double alpha_wild_right = sparkles_wild_right.best_overall();
    double alpha_own = sparkles_own.best_overall();

    {
        std::chrono::milliseconds dialog_duration = dialog_tracker.wild_animation_duration();
        std::chrono::milliseconds min_delay = SHINY_ANIMATION_DELAY - std::chrono::milliseconds(300);
        std::chrono::milliseconds max_delay = SHINY_ANIMATION_DELAY + std::chrono::milliseconds(500);
        if (min_delay <= dialog_duration && dialog_duration <= max_delay){
            alpha_wild_overall += 3.5;
        }
    }
    {
        std::chrono::milliseconds dialog_duration = dialog_tracker.your_animation_duration();
        std::chrono::milliseconds min_delay = SHINY_ANIMATION_DELAY - std::chrono::milliseconds(300);
        std::chrono::milliseconds max_delay = SHINY_ANIMATION_DELAY + std::chrono::milliseconds(2000);  //  Add headroom for happiness.
        if (min_delay <= dialog_duration && dialog_duration <= max_delay){
            alpha_own += 3.5;
        }
    }
    logger.log(
        "ShinyDetector: Wild Alpha = " + QString::number(alpha_wild_overall) +
        ", Left Alpha = " + QString::number(alpha_wild_left) +
        ", Right Alpha = " + QString::number(alpha_wild_right) +
        ", Your Alpha = " + QString::number(alpha_own),
        COLOR_PURPLE
    );

    wild_result.shiny_type = ShinyType::UNKNOWN;
    wild_result.left_is_shiny = false;
    wild_result.right_is_shiny = false;

    if (alpha_wild_overall < overall_threshold){
        logger.log("ShinyDetector: Wild not Shiny.", COLOR_PURPLE);
        wild_result.shiny_type = ShinyType::NOT_SHINY;
    }else{
        logger.log("ShinyDetector: Detected Wild Shiny!", COLOR_BLUE);
        wild_result.shiny_type = ShinyType::UNKNOWN_SHINY;
        wild_result.left_is_shiny = alpha_wild_left >= doubles_threshold;
        wild_result.right_is_shiny = alpha_wild_right >= doubles_threshold;
    }

    if (alpha_own < overall_threshold){
        logger.log("ShinyDetector: Lead not Shiny.", COLOR_PURPLE);
        your_result.shiny_type = ShinyType::NOT_SHINY;
    }else{
        logger.log("ShinyDetector: Detected Lead Shiny!", COLOR_BLUE);
        your_result.shiny_type = ShinyType::UNKNOWN_SHINY;
    }
}


void detect_shiny_battle(
    ProgramEnvironment& env, ConsoleHandle& console,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    const DetectionType& type,
    std::chrono::seconds timeout,
    double overall_threshold, double doubles_threshold
){
    BattleType battle_type = type.full_battle_menu ? BattleType::STANDARD : BattleType::STARTER;
    ShinyEncounterTracker tracker(console, console, battle_type);
    int result = wait_until(
        env, console, timeout,
        { &tracker }
    );
    if (result < 0){
        env.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
        return;
    }
    determine_shiny_status(
        console,
        wild_result, your_result,
        tracker.dialog_tracker(),
        tracker.sparkles_wild_overall(),
        tracker.sparkles_wild_left(),
        tracker.sparkles_wild_right(),
        tracker.sparkles_own()
    );
    wild_result.best_screenshot = tracker.sparkles_wild_overall().best_image();
    your_result.best_screenshot = tracker.sparkles_own().best_image();
//    your_result.best_screenshot.save("test.png");
}










}
}
}
