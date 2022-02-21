/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


ButtonMatcher::ButtonMatcher(ButtonType type)
    : WaterfillTemplateMatcher(
        type == ButtonType::ButtonA ? "PokemonLA/Buttons/ButtonA-Template.png" :
        type == ButtonType::ButtonB ? "PokemonLA/Buttons/ButtonB-Template.png" : "",
        Color(0xff808008), Color(0xffffffff), 100
    )
{}
const ButtonMatcher& ButtonMatcher::A(){
    static ButtonMatcher matcher(ButtonType::ButtonA);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::B(){
    static ButtonMatcher matcher(ButtonType::ButtonB);
    return matcher;
}





ButtonTracker::ButtonTracker(ButtonType type)
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
            Color(0xff808080),
            Color(0xff909090),
            Color(0xffa0a0a0),
            Color(0xffb0b0b0),
        }
    )
    , m_matcher(
        type == ButtonType::ButtonA ? ButtonMatcher::A() : ButtonMatcher::B()
    )
{}

void ButtonTracker::process_object(const QImage& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    image.save("test-" + QString::number(c++) + "-A.png");
//    extract_box(image, object).save("test-" + QString::number(c++) + "-B.png");

    double rmsd = m_matcher.rmsd(extract_box(image, object));
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd < 80){
//        cout << "rmsd = " << rmsd << endl;
        m_detections.emplace_back(object);
    }
}
void ButtonTracker::finish(){
    merge_heavily_overlapping();
}



ButtonDetector::ButtonDetector(
    Logger& logger, VideoOverlay& overlay,
    ButtonType type,
    const ImageFloatBox& box,
    std::chrono::milliseconds min_streak,
    bool stop_on_detected
)
    : VisualInferenceCallback("CenterAButtonDetector")
    , m_logger(logger)
    , m_box(box)
    , m_min_streak(min_streak)
    , m_stop_on_detected(stop_on_detected)
    , m_tracker(type)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_last_flip(std::chrono::system_clock::now())
    , m_current_streak(false)
    , m_detected(false)
{}

void ButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ButtonDetector::process_frame(
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
