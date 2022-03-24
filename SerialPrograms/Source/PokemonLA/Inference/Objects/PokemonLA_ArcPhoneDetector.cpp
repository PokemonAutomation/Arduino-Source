/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
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

void ArcPhoneTracker::process_object(const ConstImageRef& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    cout << "c = " << c << endl;
//    extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    image.save("test-" + QString::number(c++) + "-A.png");
//    extract_box(image, object).save("test-" + QString::number(c++) + "-B.png");

    double width = (double)object.width() / image.width();
    if (width < 0.40 || width > 0.50){
        return;
    }

//    cout << (double)object.width() / image.width() << endl;

    double rmsd = ArcPhoneMatcher::instance().rmsd_original(image, object);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd < 80){
//        cout << "rmsd = " << rmsd << endl;
//         extract_box(image, object).save("test.png");
        m_detections.emplace_back(object);
    }
}
void ArcPhoneTracker::finish(){
    merge_heavily_overlapping();
}



ArcPhoneDetector::ArcPhoneDetector(
    LoggerQt& logger, VideoOverlay& overlay,
    std::chrono::milliseconds min_streak,
    bool stop_on_detected
)
    : VisualInferenceCallback("CenterAButtonDetector")
    , m_logger(logger)
    , m_box(0.010, 0.700, 0.050, 0.100)
    , m_stop_on_detected(stop_on_detected)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_debouncer(
        false, min_streak,
        [&](bool value){
            if (value){
                m_logger.log("Detected Arc Phone.", COLOR_PURPLE);
            }else{
                m_logger.log("Arc Phone has disappeared.", COLOR_PURPLE);
            }
        }
    )
{}


void ArcPhoneDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ArcPhoneDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    m_watcher.process_frame(frame, timestamp);
    bool detected = m_debouncer.push_value(!m_tracker.detections().empty(), timestamp);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("ArcPhoneTriggered-" + QString::number(c++) + ".png");
    }
#endif

    return detected && m_stop_on_detected;
}





}
}
}
