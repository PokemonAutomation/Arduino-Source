/*  Battle Sprite Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "PokemonLA_BattleSpriteArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattleSpriteArrowMatcher::BattleSpriteArrowMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/BattleSpriteArrow-Template.png",
        Color(0xff808008), Color(0xffffffff), 100
    )
{
    m_aspect_ratio_lower = 0.95;
    m_aspect_ratio_upper = 1.05;
    m_area_ratio_lower = 0.95;
    m_area_ratio_upper = 1.05;
}

const BattleSpriteArrowMatcher& BattleSpriteArrowMatcher::instance(){
    static BattleSpriteArrowMatcher matcher;
    return matcher;
}


BattleSpriteArrowTracker::BattleSpriteArrowTracker()
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
            Color(0xff808080),
            Color(0xff909090),
            Color(0xffa0a0a0),
            Color(0xffb0b0b0),
        }
    )
{}

void BattleSpriteArrowTracker::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
    double rmsd = BattleSpriteArrowMatcher::instance().rmsd_original(image, object);
    if (rmsd < 80){
        m_detections.emplace_back(object);
    }
}

void BattleSpriteArrowTracker::finish(const ImageViewRGB32& image){
    merge_heavily_overlapping();
}



BattleSpriteArrowDetector::BattleSpriteArrowDetector(
    Logger& logger, VideoOverlay& overlay,
    size_t sprite_index,
    std::chrono::milliseconds min_streak,
    bool stop_on_detected
)
    : VisualInferenceCallback("BattleSpriteArrowButtonDetector")
    , m_logger(logger)
    , m_box(0.936 - 0.035*sprite_index, 0.018, 0.015, 0.027)
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_debouncer(
        false, min_streak,
        [&](bool value){
            if (value){
                m_logger.log("Detected sprite arrow.", COLOR_PURPLE);
            }else{
                m_logger.log("Sprite arrow disappeared.", COLOR_PURPLE);
            }
        }
    )
{}


void BattleSpriteArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool BattleSpriteArrowDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);
    bool detected = m_debouncer.push_value(!m_tracker.detections().empty(), timestamp);
    return detected && m_stop_on_detected;
}





}
}
}
