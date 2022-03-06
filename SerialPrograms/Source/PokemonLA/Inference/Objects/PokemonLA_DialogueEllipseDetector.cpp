/*  Dialogue Ellipse Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonLA_DialogueEllipseDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


DialogueEllipseMatcher::DialogueEllipseMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/DialogueEllipse-Template.png",
        Color(0xff808008), Color(0xffffffff), 300
    )
{
    m_aspect_ratio_lower = 0.95;
    m_aspect_ratio_upper = 1.05;
    m_area_ratio_lower = 0.95;
    m_area_ratio_upper = 1.05;
}

const DialogueEllipseMatcher& DialogueEllipseMatcher::instance(){
    static DialogueEllipseMatcher matcher;
    return matcher;
}


DialogueEllipseTracker::DialogueEllipseTracker()
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

void DialogueEllipseTracker::process_object(const QImage& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    cout << "c = " << c << endl;
//    extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    image.save("test-" + QString::number(c++) + "-A.png");
//    extract_box(image, object).save("test-" + QString::number(c++) + "-B.png");

    double rmsd = DialogueEllipseMatcher::instance().rmsd(image, object);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd < 80){
//        cout << "rmsd = " << rmsd << endl;
        m_detections.emplace_back(object);
    }
}
void DialogueEllipseTracker::finish(){
    merge_heavily_overlapping();
}



DialogueEllipseDetector::DialogueEllipseDetector(
    LoggerQt& logger, VideoOverlay& overlay,
    std::chrono::milliseconds min_streak,
    bool stop_on_detected
)
    : VisualInferenceCallback("CenterAButtonDetector")
    , m_logger(logger)
    , m_box(0.741, 0.811, 0.028, 0.023)
    , m_min_streak(min_streak)
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_last_flip(std::chrono::system_clock::now())
    , m_current_streak(false)
    , m_detected(false)
{}


void DialogueEllipseDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool DialogueEllipseDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    m_watcher.process_frame(frame, timestamp);

    SpinLockGuard lg(m_lock);

    //  Streak ends
    if (m_current_streak == m_tracker.detections().empty()){
        m_current_streak = !m_tracker.detections().empty();
        m_last_flip = timestamp;
        return false;
    }

    //  Streak not long enough.
    if (timestamp - m_last_flip < m_min_streak){
        return false;
    }

    //  No state change.
    if (m_current_streak == detected()){
        return false;
    }

    if (m_current_streak){
        m_logger.log("Detected transparent dialogue box ellipse button.", COLOR_PURPLE);
    }else{
        m_logger.log("Transparent dialogue box ellipse button disappeared.", COLOR_PURPLE);
    }
    m_detected.store(m_current_streak, std::memory_order_release);

    return m_stop_on_detected;
}





}
}
}
