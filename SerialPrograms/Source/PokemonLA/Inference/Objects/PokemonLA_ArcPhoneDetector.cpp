/*  Arc Phone Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonLA_ArcPhoneDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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

void ArcPhoneTracker::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    cout << "c = " << c << endl;
//    extract_box_reference(image, object).save("test-" + std::to_string(c++) + ".png");
//    image.save("test-" + std::to_string(c++) + "-A.png");
//    extract_box(image, object).save("test-" + std::to_string(c++) + "-B.png");

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
void ArcPhoneTracker::finish(const ImageViewRGB32& image){
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
    , m_stop_on_detected(stop_on_detected)
    , m_tracker_button(ButtonType::ButtonMinus)
    , m_watcher(
        overlay, m_box, {
            {m_tracker_phone, false},
            {m_tracker_button, false},
        }
    )
    , m_debouncer_phone(
        false, min_streak,
        [&](bool value){
            if (value){
                m_logger.log("Detected Arc Phone.", COLOR_PURPLE);
            }else{
                m_logger.log("Arc Phone has disappeared.", COLOR_PURPLE);
            }
        }
    )
    , m_debouncer_button(
        false, min_streak,
        [&](bool value){
            if (value){
                m_logger.log("Detected (-) Button.", COLOR_PURPLE);
            }else{
                m_logger.log("(-) button has disappeared.", COLOR_PURPLE);
            }
        }
    )
{}


void ArcPhoneDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ArcPhoneDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);
    bool detected0 = m_debouncer_phone.push_value(!m_tracker_phone.detections().empty(), timestamp);
    bool detected1 = m_debouncer_button.push_value(!m_tracker_phone.detections().empty(), timestamp);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("ArcPhoneTriggered-" + std::to_string(c++) + ".png");
    }
#endif

    return detected0 && detected1 && m_stop_on_detected;
}





}
}
}
