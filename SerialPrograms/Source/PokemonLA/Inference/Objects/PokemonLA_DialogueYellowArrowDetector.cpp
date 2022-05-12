/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonLA_DialogueYellowArrowDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


DialogueYellowArrowMatcher::DialogueYellowArrowMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/YellowArrow-Template.png",
        Color(0xff808008), Color(0xffffffff), 200
    )
{
    m_aspect_ratio_lower = 0.9;
    m_aspect_ratio_upper = 1.1;
    m_area_ratio_lower = 0.9;
    m_area_ratio_upper = 1.1;
}

const DialogueYellowArrowMatcher& DialogueYellowArrowMatcher::instance(){
    static DialogueYellowArrowMatcher matcher;
    return matcher;
}

DialogueYellowArrowTracker::DialogueYellowArrowTracker()
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
            Color(0xff808000),
            Color(0xff909000),
            Color(0xffa0a000),
            Color(0xffb0b000),
        }
    )
{}

void DialogueYellowArrowTracker::process_object(const ConstImageRef& image, const WaterfillObject& object){
    double rmsd = DialogueYellowArrowMatcher::instance().rmsd_original(image, object);
    if (rmsd < 80){
        m_detections.emplace_back(object);
    }
}

void DialogueYellowArrowTracker::finish(){
    merge_heavily_overlapping();
}



DialogueYellowArrowDetector::DialogueYellowArrowDetector(
    LoggerQt& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("DialogueYellowArrowDetector")
    , m_logger(logger)
    , m_box(0.741, 0.759, 0.028, 0.062)
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(
        overlay, m_box, {{m_tracker, false}}
    )
{}


void DialogueYellowArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool DialogueYellowArrowDetector::process_frame(const QImage& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);
    const bool detected = !m_tracker.detections().empty();
    if (detected){
        m_logger.log("Detected yellow arrow in transparent dialogue box.", COLOR_PURPLE);
    }

    m_detected.store(detected, std::memory_order_release);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("YellowArrowTriggered-" + QString::number(c++) + ".png");
    }
#endif

    return detected && m_stop_on_detected;
}





}
}
}
