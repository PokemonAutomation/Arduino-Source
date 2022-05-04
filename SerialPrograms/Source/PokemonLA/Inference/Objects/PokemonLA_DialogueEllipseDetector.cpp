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

void DialogueEllipseTracker::process_object(const ConstImageRef& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    cout << "c = " << c << endl;
//    extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    image.save("test-" + QString::number(c++) + "-A.png");
//    extract_box(image, object).save("test-" + QString::number(c++) + "-B.png");

    double rmsd = DialogueEllipseMatcher::instance().rmsd_original(image, object);
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
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_debouncer(
        false, min_streak,
        [&](bool value){
            if (value){
                m_logger.log("Detected transparent dialogue box ellipse button.", COLOR_PURPLE);
            }else{
                m_logger.log("Transparent dialogue box ellipse button disappeared.", COLOR_PURPLE);
            }
        }
    )
{}


void DialogueEllipseDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool DialogueEllipseDetector::process_frame(const QImage& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);
    bool detected = m_debouncer.push_value(!m_tracker.detections().empty(), timestamp);

    // static int count = 0;
    // if (detected){
    //     frame.save("./debug_transparentEllipse" + QString::number(count) + ".png");
    //     count++;
    // }
    return detected && m_stop_on_detected;
}





}
}
}
