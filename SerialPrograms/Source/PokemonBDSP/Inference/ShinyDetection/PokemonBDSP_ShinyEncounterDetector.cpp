/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Sounds/PokemonBDSP_ShinySoundDetector.h"
#include "PokemonBDSP_ShinyEncounterDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
    Logger& logger, VideoOverlay& overlay,
    BattleType battle_type
)
    : VisualInferenceCallback("ShinyEncounterTracker")
    , m_logger(logger)
//    , m_overlay(overlay)
    , m_battle_menu(battle_type)
    , m_dialog_tracker(logger, m_dialog_detector)
    , m_wild_animation_end_timestamp(WallClock::min())
    , m_your_animation_start_timestamp(WallClock::min())
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
bool ShinyEncounterTracker::process_frame(const VideoSnapshot& frame){
    using PokemonSwSh::EncounterState;

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

    // End shiny detection when we reach the battle menu
    bool battle_menu = m_battle_menu.process_frame(frame);
    if (battle_menu){
        m_dialog_tracker.push_end(frame.timestamp);
        return true;
    }

    // Use m_dialog_tracker to track dialog timings. Dialogs partitions the opening of a battle into:
    // before anything -> wild pokemon animation -> player pokemon animation -> battle menu detected
    m_dialog_tracker.process_frame(frame);

    switch (m_dialog_tracker.encounter_state()){
    case EncounterState::BEFORE_ANYTHING:
        break;
    case EncounterState::WILD_ANIMATION:{
        //  Update the timestamp that records the end of wild animation
        m_wild_animation_end_timestamp = frame.timestamp;

        m_sparkle_tracker_wild.process_frame(frame);
        m_sparkle_tracker_own.clear_boxes();
        m_best_wild_overall.add_frame(frame.frame, m_sparkles_wild);

        ImagePixelBox box_overall = floatbox_to_pixelbox(width, height, {0.4, 0.02, 0.60, 0.93});
        ImagePixelBox box_left = floatbox_to_pixelbox(width, height, m_box_wild_left);
        ImagePixelBox box_right = floatbox_to_pixelbox(width, height, m_box_wild_right);
        box_left.clip(box_overall);
        box_right.clip(box_overall);
        box_left.min_x -= box_overall.min_x;
        box_left.min_y -= box_overall.min_y;
        box_left.max_x -= box_overall.min_x;
        box_left.max_y -= box_overall.min_y;
        box_right.min_x -= box_overall.min_x;
        box_right.min_y -= box_overall.min_y;
        box_right.max_x -= box_overall.min_x;
        box_right.max_y -= box_overall.min_y;

        m_best_wild_left.add_frame(nullptr, m_sparkles_wild.extract_subbox(box_left));
        m_best_wild_right.add_frame(nullptr, m_sparkles_wild.extract_subbox(box_right));
        break;
    }
    case EncounterState::YOUR_ANIMATION:
        //  Update the timestamp that records the start of player pokemon animation
        if (m_your_animation_start_timestamp == WallClock::min()){
            m_your_animation_start_timestamp = frame.timestamp;
        }

        m_sparkle_tracker_wild.clear_boxes();
        m_sparkle_tracker_own.process_frame(frame);
        m_best_own.add_frame(frame.frame, m_sparkles_own);
        break;
    case EncounterState::POST_ENTRY:
        break;
    }

    return false;
}

void determine_shiny_status(
    ProgramEnvironment& env,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    EventNotificationOption& settings,
    BattleType battle_type,
    const ShinyEncounterTracker& tracker,
    const std::vector<WallClock>& shiny_sound_timestamps
){
    const double OVERALL_THRESHOLD = GameSettings::instance().SHINY_ALPHA_OVERALL_THRESHOLD;
    const double DOUBLES_THRESHOLD = GameSettings::instance().SHINY_ALPHA_SIDE_THRESHOLD;
    const double DIALOG_ALPHA = GameSettings::instance().SHINY_DIALOG_ALPHA;
    const double SOUND_ALPHA = GameSettings::instance().SHINY_SOUND_ALPHA;

    const PokemonSwSh::EncounterDialogTracker& dialog_tracker = tracker.dialog_tracker();
    const ShinySparkleAggregator& sparkles_wild_overall = tracker.sparkles_wild_overall();
    const ShinySparkleAggregator& sparkles_wild_left = tracker.sparkles_wild_left();
    const ShinySparkleAggregator& sparkles_wild_right = tracker.sparkles_wild_right();
    const ShinySparkleAggregator& sparkles_own = tracker.sparkles_own();

    double alpha_wild_overall = sparkles_wild_overall.best_overall();
    double alpha_wild_left = sparkles_wild_left.best_overall();
    double alpha_wild_right = sparkles_wild_right.best_overall();
    double alpha_own = sparkles_own.best_overall();

    {
        std::chrono::milliseconds dialog_duration = dialog_tracker.wild_animation_duration();
        std::chrono::milliseconds min_delay = SHINY_ANIMATION_DELAY - std::chrono::milliseconds(300);
        std::chrono::milliseconds max_delay = SHINY_ANIMATION_DELAY + std::chrono::milliseconds(500);
        if (min_delay <= dialog_duration && dialog_duration <= max_delay){
            alpha_wild_overall += DIALOG_ALPHA;
        }
    }

#if 0
    cout << "Wild End: " << std::chrono::duration_cast<Milliseconds>(tracker.wild_animation_end_timestmap() - tracker.m_start_time) << endl;
    cout << "Your Start: " << std::chrono::duration_cast<Milliseconds>(tracker.your_animation_start_timestamp() - tracker.m_start_time) << endl;
    for (WallClock time : shiny_sound_timestamps){
        cout << "Shiny Sound: " << std::chrono::duration_cast<Milliseconds>(time - tracker.m_start_time) << endl;
    }
#endif

    bool wild_shiny_sound_detected = false;
    bool own_shiny_sound_detected = false;
    for (const WallClock& timestamp: shiny_sound_timestamps){
        const WallClock& wild_end = tracker.wild_animation_end_timestmap();
        const WallClock& own_start = tracker.your_animation_start_timestamp();

        bool wild_shiny = timestamp < wild_end + Milliseconds(500);
        bool own_shiny = timestamp >= own_start;

        wild_shiny_sound_detected |= wild_shiny;
        own_shiny_sound_detected |= own_shiny;
        if (!wild_shiny && !own_shiny){
            throw_and_log<OperationFailedException>(
                env.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Wrong shiny sound timing found."
            );
        }
    }
    alpha_wild_overall += wild_shiny_sound_detected ? SOUND_ALPHA : 0.0;
    alpha_own += own_shiny_sound_detected ? SOUND_ALPHA : 0.0;

    if (battle_type == BattleType::STARTER){
        std::chrono::milliseconds dialog_duration = dialog_tracker.your_animation_duration();
        std::chrono::milliseconds min_delay = SHINY_ANIMATION_DELAY - std::chrono::milliseconds(300);
        std::chrono::milliseconds max_delay = SHINY_ANIMATION_DELAY + std::chrono::milliseconds(2000);  //  Add headroom for happiness.
        if (min_delay <= dialog_duration && dialog_duration <= max_delay){
            alpha_own += DIALOG_ALPHA;
        }
    }
    env.log(
        "ShinyDetector: Wild Alpha = " + tostr_default(alpha_wild_overall) +
        (wild_shiny_sound_detected ? " (shiny sound detected)" : "") +
        ", Left Alpha = " + tostr_default(alpha_wild_left) +
        ", Right Alpha = " + tostr_default(alpha_wild_right) +
        ", Your Alpha = " + tostr_default(alpha_own) +
        (own_shiny_sound_detected ? " (shiny sound detected)" : ""),
        COLOR_PURPLE
    );

    wild_result.shiny_type = ShinyType::UNKNOWN;
    wild_result.alpha = alpha_wild_overall;
    wild_result.left_is_shiny = false;
    wild_result.right_is_shiny = false;

    if (alpha_wild_overall < OVERALL_THRESHOLD){
        env.log("ShinyDetector: Wild not Shiny.", COLOR_PURPLE);
        wild_result.shiny_type = ShinyType::NOT_SHINY;
    }else{
        env.log("ShinyDetector: Detected Wild Shiny!", COLOR_BLUE);
        wild_result.shiny_type = ShinyType::UNKNOWN_SHINY;
        wild_result.left_is_shiny = alpha_wild_left >= DOUBLES_THRESHOLD;
        wild_result.right_is_shiny = alpha_wild_right >= DOUBLES_THRESHOLD;
    }

    if (DIALOG_ALPHA <= alpha_wild_overall && alpha_wild_overall < DIALOG_ALPHA + 1.5){
        dump_image(env.logger(), env.program_info(), "LowShinyAlpha", wild_result.get_best_screenshot());
        std::string str;
        str += "Low alpha shiny (alpha = ";
        str += tostr_default(alpha_wild_overall);
        str += ").\nPlease report this image to the " + PROGRAM_NAME + " server.";
        send_program_recoverable_error_notification(
            env, settings,
            str,
            wild_result.get_best_screenshot()
        );
    }

    your_result.alpha = alpha_own;
    if (alpha_own < OVERALL_THRESHOLD){
        env.log("ShinyDetector: Lead not Shiny.", COLOR_PURPLE);
        your_result.shiny_type = ShinyType::NOT_SHINY;
    }else{
        env.log("ShinyDetector: Detected Lead Shiny!", COLOR_BLUE);
        your_result.shiny_type = ShinyType::UNKNOWN_SHINY;
    }
}


void detect_shiny_battle(
    ProgramEnvironment& env,
    VideoStream& stream, CancellableScope& scope,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    EventNotificationOption& settings,
    const DetectionType& type,
    std::chrono::seconds timeout,
    bool use_shiny_sound
){
    BattleType battle_type = type.full_battle_menu ? BattleType::STANDARD : BattleType::STARTER;
    ShinyEncounterTracker tracker(stream.logger(), stream.overlay(), battle_type);

    std::unique_ptr<ShinySoundDetector> shiny_sound_detector;
    std::vector<WallClock> shiny_sound_timestamps; // the times where shiny sound is detected
    
    if (use_shiny_sound){
        shiny_sound_detector = std::make_unique<ShinySoundDetector>(
            stream.logger(),
            [&shiny_sound_timestamps](float error_coefficient) -> bool{
                //  Warning: This callback will be run from a different thread than this function.
                //  When this lambda function is called, a shiny sound is detected.
                //  Mark this by `shiny_sound_timestamps`.
                shiny_sound_timestamps.emplace_back(current_time());
                //  This lambda function always returns false. It tells the shiny sound detector to always return false
                //  in ShinySoundDetector::process_spectrums() when a shiny sound is found, so that it won't stop
                //  ShinyEncounterTracker tracker from finish running.

                return false;
            }
        );
    }

    std::vector<PeriodicInferenceCallback> callbacks = {{tracker}};
    if (use_shiny_sound){
        callbacks.emplace_back(*shiny_sound_detector);
    }
    int result = wait_until(stream, scope, timeout, callbacks);
    if (result < 0){
        stream.log("ShinyDetector: Battle menu not found after timeout.", COLOR_RED);
        return;
    }
    wild_result.best_screenshot = tracker.sparkles_wild_overall().best_image();
    your_result.best_screenshot = tracker.sparkles_own().best_image();
//    your_result.best_screenshot.save("test.png");
    determine_shiny_status(
        env,
        wild_result, your_result,
        settings,
        battle_type,
        tracker,
        shiny_sound_timestamps
    );
}










}
}
}
