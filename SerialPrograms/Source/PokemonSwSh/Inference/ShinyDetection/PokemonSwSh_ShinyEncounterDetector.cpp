/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

//#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const ShinyDetectionBattle SHINY_BATTLE_REGULAR {false, {0.5, 0.05, 0.5, 0.70}, std::chrono::milliseconds(2300)};
const ShinyDetectionBattle SHINY_BATTLE_RAID    {true,  {0.3, 0.01, 0.7, 0.75}, std::chrono::milliseconds(3900)};



ShinyEncounterTracker::ShinyEncounterTracker(
    LoggerQt& logger, VideoOverlay& overlay,
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
    LoggerQt& logger,
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


ShinyDetectionResult detect_shiny_battle(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold
){
    ShinyEncounterTracker tracker(console, console, battle_settings);
    int result = wait_until(
        env, context, console, timeout,
        { &tracker }
    );
    if (result < 0){
        env.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
        return ShinyDetectionResult{ShinyType::UNKNOWN, QImage()};
    }
    ShinyType shiny_type = determine_shiny_status(
        console,
        battle_settings,
        tracker.dialog_tracker(),
        tracker.sparkles_wild()
    );
    return ShinyDetectionResult{shiny_type, tracker.sparkles_wild().best_image()};
}





}
}
}
