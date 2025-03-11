/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "PokemonLA_ButtonDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace{

const char* templatePath(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return "PokemonLA/Buttons/ButtonA-Template.png";
    case ButtonType::ButtonB:
        return "PokemonLA/Buttons/ButtonB-Template.png";
    case ButtonType::ButtonPlus:
        return "PokemonLA/Buttons/ButtonPlus-Template.png";
    case ButtonType::ButtonMinus:
        return "PokemonLA/Buttons/ButtonMinus-Template.png";
    case ButtonType::ArrowLeft:
        return "PokemonLA/Buttons/ArrowLeft-Template.png";
    case ButtonType::ArrowRight:
        return "PokemonLA/Buttons/ArrowRight-Template.png";
    default:
        return "";
    }
}

const char* button_name(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return "A";
    case ButtonType::ButtonB:
        return "B";
    case ButtonType::ButtonPlus:
        return "+";
    case ButtonType::ButtonMinus:
        return "-";
    case ButtonType::ArrowLeft:
        return "<";
    case ButtonType::ArrowRight:
        return ">";
    default:
        return "";
    }
}

const ButtonMatcher& getButtonMatcher(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return ButtonMatcher::A();
    case ButtonType::ButtonB:
        return ButtonMatcher::B();
    case ButtonType::ButtonPlus:
        return ButtonMatcher::Plus();
    case ButtonType::ButtonMinus:
        return ButtonMatcher::Minus();
    case ButtonType::ArrowLeft:
        return ButtonMatcher::ArrowLeft();
    case ButtonType::ArrowRight:
        return ButtonMatcher::ArrowRight();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for ButtonType");
    }
}

}

ButtonMatcher::ButtonMatcher(ButtonType type, size_t min_width, size_t max_width, double max_rmsd)
    : WaterfillTemplateMatcher(
        templatePath(type), Color(0xff808008), Color(0xffffffff), 100
    )
    , m_min_width(min_width)
    , m_min_height(max_width)
    , m_max_rmsd(max_rmsd)
{}
const ButtonMatcher& ButtonMatcher::A(){
    static ButtonMatcher matcher(ButtonType::ButtonA, 15, 15, 90);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::B(){
    static ButtonMatcher matcher(ButtonType::ButtonB, 15, 15, 90);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::Plus(){
    static ButtonMatcher matcher(ButtonType::ButtonPlus, 15, 15, 120);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::Minus(){
    static ButtonMatcher matcher(ButtonType::ButtonMinus, 15, 15, 120);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::ArrowLeft(){
    static ButtonMatcher matcher(ButtonType::ArrowLeft, 5, 10, 180);
    return matcher;
}
const ButtonMatcher& ButtonMatcher::ArrowRight(){
    static ButtonMatcher matcher(ButtonType::ArrowRight, 5, 10, 180);
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
            Color(0xffc0c0c0),
            Color(0xffd0d0d0),
        }
    )
    , m_matcher(getButtonMatcher(type))
{}

void ButtonTracker::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
//    cout << "asdf" << endl;
//    static int c = 0;
//    extract_box(image, object).save("test-" + std::to_string(c++) + ".png");
//    image.save("test-" + std::to_string(c++) + "-A.png");
//    extract_box(image, object).save("test-" + std::to_string(c++) + "-B.png");

    double rmsd = m_matcher.rmsd_precropped(extract_box_reference(image, object), object);
//    cout << "rmsd = " << rmsd << endl;
//    cout << "max = " << m_matcher.m_max_rmsd << endl;
    if (rmsd < m_matcher.m_max_rmsd){
//        cout << "matching rmsd = " << rmsd << endl;
        m_detections.emplace_back(object);
    }
}
void ButtonTracker::finish(const ImageViewRGB32& image){
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
    , m_stop_on_detected(stop_on_detected)
    , m_tracker(type)
    , m_watcher(overlay, m_box, { {m_tracker, false} })
    , m_debouncer(
        false, min_streak,
        [this, type](bool value){
            if (value){
                m_logger.log(std::string("Detected (") + button_name(type) + ") Button.", COLOR_PURPLE);
            }else{
                m_logger.log(std::string("(") + button_name(type) + ") Button has disappeared.", COLOR_PURPLE);
            }
        }
    )
{}

void ButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ButtonDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);
    bool detected = m_debouncer.push_value(!m_tracker.detections().empty(), timestamp);
    return detected && m_stop_on_detected;
}




}
}
}
