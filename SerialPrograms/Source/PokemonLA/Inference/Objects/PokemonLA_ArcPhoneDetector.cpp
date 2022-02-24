/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonLA_ArcPhoneDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


ArcPhoneMatcher::ArcPhoneMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/ArcPhone-Template.png",
        Color(0xff808008), Color(0xffffffff), 100
    )
{}
const ArcPhoneMatcher& ArcPhoneMatcher::instance(){
    static ArcPhoneMatcher matcher;
    return matcher;
}


ArcPhoneTracker::ArcPhoneTracker()
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

void ArcPhoneTracker::process_object(const QImage& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    cout << "c = " << c << endl;
//    extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    image.save("test-" + QString::number(c++) + "-A.png");
//    extract_box(image, object).save("test-" + QString::number(c++) + "-B.png");

    double rmsd = ArcPhoneMatcher::instance().rmsd(image, object);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd < 80){
//        cout << "rmsd = " << rmsd << endl;
        m_detections.emplace_back(object);
    }
}
void ArcPhoneTracker::finish(){
    merge_heavily_overlapping();
}



ArcPhoneDetector::ArcPhoneDetector(
    Logger& logger, VideoOverlay& overlay,
    std::chrono::milliseconds min_streak,
    bool stop_on_detected
)
    : VisualInferenceCallback("CenterAButtonDetector")
    , m_logger(logger)
    , m_box(0.010, 0.700, 0.050, 0.100)
    , m_min_streak(min_streak)
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_last_flip(std::chrono::system_clock::now())
    , m_current_streak(false)
    , m_detected(false)
{}


void ArcPhoneDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ArcPhoneDetector::process_frame(
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
        m_logger.log("Detected (A) Button.", COLOR_PURPLE);
    }else{
        m_logger.log("(A) Button has disappeared.", COLOR_PURPLE);
    }
    m_detected.store(m_current_streak, std::memory_order_release);

    return m_stop_on_detected;
}





}
}
}
