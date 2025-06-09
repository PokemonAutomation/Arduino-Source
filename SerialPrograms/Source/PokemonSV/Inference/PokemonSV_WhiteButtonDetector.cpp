/*  White Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
//#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSV_WhiteButtonDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels;
using namespace Kernels::Waterfill;




const char* white_button_template_path(WhiteButton type){
    switch (type){
    case WhiteButton::ButtonA:
        return "PokemonSV/Buttons/WhiteButtonA.png";
    case WhiteButton::ButtonA_DarkBackground:
        return "PokemonSV/Buttons/WhiteButtonA-dark.png";        
    case WhiteButton::ButtonB:
        return "PokemonSV/Buttons/WhiteButtonB.png";
    case WhiteButton::ButtonX:
        return "PokemonSV/Buttons/WhiteButtonX.png";
    case WhiteButton::ButtonY:
        return "PokemonSV/Buttons/WhiteButtonY.png";
    case WhiteButton::ButtonMinus:
        return "PokemonSV/Buttons/WhiteButtonMinus.png";
    case WhiteButton::ButtonLeft:
        return "PokemonSV/Buttons/ArrowLeft.png";
    case WhiteButton::ButtonRight:
        return "PokemonSV/Buttons/ArrowRight.png";
    case WhiteButton::ButtonLStick:
        return "PokemonSV/Buttons/LStick.png";
    default:
        return "";
    }
}

const WhiteButtonMatcher& get_button_matcher(WhiteButton type){
    switch (type){
    case WhiteButton::ButtonA:
        return WhiteButtonMatcher::A();
    case WhiteButton::ButtonA_DarkBackground:
        return WhiteButtonMatcher::A_DarkBackground();        
    case WhiteButton::ButtonB:
        return WhiteButtonMatcher::B();
    case WhiteButton::ButtonX:
        return WhiteButtonMatcher::X();
    case WhiteButton::ButtonY:
        return WhiteButtonMatcher::Y();
    case WhiteButton::ButtonMinus:
        return WhiteButtonMatcher::Minus();
    case WhiteButton::ButtonLeft:
        return WhiteButtonMatcher::ArrowLeft();
    case WhiteButton::ButtonRight:
        return WhiteButtonMatcher::ArrowRight();
    case WhiteButton::ButtonLStick:
        return WhiteButtonMatcher::LStick();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for WhiteButton");
    }
}




WhiteButtonMatcher::WhiteButtonMatcher(WhiteButton type, size_t min_width, size_t max_width, double max_rmsd)
    : WaterfillTemplateMatcher(
        white_button_template_path(type), Color(0xff808008), Color(0xffffffff), 100
    )
    , m_min_width(min_width)
    , m_min_height(max_width)
    , m_max_rmsd(max_rmsd)
{}
const WhiteButtonMatcher& WhiteButtonMatcher::A(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonA, 15, 15, 100);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::A_DarkBackground(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonA_DarkBackground, 15, 15, 100);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::B(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonB, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::X(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonX, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::Y(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonY, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::Minus(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonMinus, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::ArrowLeft(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonLeft, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::ArrowRight(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonRight, 15, 15, 90);
    return matcher;
}
const WhiteButtonMatcher& WhiteButtonMatcher::LStick(){
    static WhiteButtonMatcher matcher(WhiteButton::ButtonLStick, 15, 15, 90);
    return matcher;
}







WhiteButtonDetector::WhiteButtonDetector(
    Color color,
    WhiteButton button,
    const ImageFloatBox& box
)
    : m_matcher(get_button_matcher(button))
    , m_color(color)
    , m_box(box)
{}
void WhiteButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool WhiteButtonDetector::detect(const ImageViewRGB32& screen){
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> WhiteButtonDetector::detect_all(const ImageViewRGB32& screen) const{
    ImageViewRGB32 region = extract_box_reference(screen, m_box);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff808080, 0xffffffff);

    std::vector<ImageFloatBox> hits;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(50);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        if (object.min_x == 0 || object.min_y == 0 ||
            object.max_x == region.width() || object.max_y == region.height()
        ){
            continue;
        }
        double rmsd = m_matcher.rmsd_original(region, object);
//        cout << "rmsd = " << rmsd << endl;
        if (rmsd < m_matcher.m_max_rmsd){
            hits.emplace_back(translate_to_parent(screen, m_box, object));
        }
    }
    return hits;
}



#if 0
WhiteButtonWatcher::~WhiteButtonWatcher() = default;
WhiteButtonWatcher::WhiteButtonWatcher(
    Color color,
    WhiteButton button, size_t consecutive_detections,
    VideoOverlay& overlay,
    const ImageFloatBox& box
)
    : VisualInferenceCallback("GradientArrowFinder")
    , m_overlay(overlay)
    , m_detector(color, button, box)
    , m_consecutive_detections(consecutive_detections)
{}

void WhiteButtonWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool WhiteButtonWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    std::vector<ImageFloatBox> hits = m_detector.detect_all(frame);
//    cout << "arrows = " << arrows.size() << endl;
    m_arrows.reset(hits.size());
    for (const ImageFloatBox& hit : hits){
        m_arrows.emplace_back(m_overlay, hit, COLOR_MAGENTA);
    }
//    if (!hits.empty()){
//        frame.save("test.png");
//    }

    if (hits.empty()){
        m_trigger_count = 0;
        return false;
    }

    m_trigger_count++;
    return m_trigger_count >= m_consecutive_detections;
}
#endif





}
}
}
