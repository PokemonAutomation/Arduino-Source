/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const ShinyDetectionBattle SHINY_BATTLE_REGULAR {false, {0.5, 0.05, 0.5, 0.70}, std::chrono::milliseconds(2300)};
const ShinyDetectionBattle SHINY_BATTLE_RAID    {true,  {0.3, 0.01, 0.7, 0.75}, std::chrono::milliseconds(3900)};



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
bool ShinyEncounterTracker::process_frame(const VideoSnapshot& frame){
    if (!frame){
        return false;
    }

    size_t width = frame->width();
    size_t height = frame->height();

    if (height < 720){
        throw UserSetupError(m_logger, "Video resolution must be at least 720p.");
    }
    double aspect_ratio = (double)width / height;
    if (aspect_ratio < 1.77 || aspect_ratio > 1.78){
        throw UserSetupError(m_logger, "Video aspect ratio must be 16:9.");
    }

    bool battle_menu = m_battle_menu.process_frame(frame);
    if (battle_menu){
        m_dialog_tracker.push_end(frame.timestamp);
        return true;
    }

    m_dialog_tracker.process_frame(frame);
    m_sparkle_tracker.process_frame(frame);

    switch (m_dialog_tracker.encounter_state()){
    case EncounterState::BEFORE_ANYTHING:
        break;
    case EncounterState::WILD_ANIMATION:
        m_best_wild.add_frame(frame.frame, m_sparkles);
        break;
    case EncounterState::YOUR_ANIMATION:
        break;
    case EncounterState::POST_ENTRY:
        break;
    }

    return false;
}

ShinyType determine_shiny_status(
    double& alpha,
    Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    const EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles
){
    alpha = sparkles.best_overall();

    std::chrono::milliseconds dialog_duration = dialog_tracker.wild_animation_duration();
    std::chrono::milliseconds min_delay = battle_settings.dialog_delay_when_shiny - std::chrono::milliseconds(300);
    std::chrono::milliseconds max_delay = battle_settings.dialog_delay_when_shiny + std::chrono::milliseconds(500);
    if (min_delay <= dialog_duration && dialog_duration <= max_delay){
        alpha += GameSettings::instance().SHINY_DIALOG_ALPHA;
    }

    double best_star = sparkles.best_star();
    double best_square = sparkles.best_square();

    logger.log(
        "ShinyDetector: Overall Alpha = " + tostr_default(alpha) +
        ", Star Alpha = " + tostr_default(best_star) +
        ", Square Alpha = " + tostr_default(best_square),
        COLOR_PURPLE
    );

    if (alpha < GameSettings::instance().SHINY_ALPHA_THRESHOLD){
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
    VideoStream& stream, CancellableScope& scope,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout
){
    ShinyEncounterTracker tracker(stream.logger(), stream.overlay(), battle_settings);
    int result = wait_until(
        stream, scope, timeout,
        {{tracker}}
    );
    if (result < 0){
        stream.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
        return ShinyDetectionResult{ShinyType::UNKNOWN, 0, std::make_shared<ImageRGB32>()};
    }
    double alpha;
    ShinyType shiny_type = determine_shiny_status(
        alpha,
        stream.logger(),
        battle_settings,
        tracker.dialog_tracker(),
        tracker.sparkles_wild()
    );
    return ShinyDetectionResult{shiny_type, alpha, tracker.sparkles_wild().best_image()};
}





}
}
}
