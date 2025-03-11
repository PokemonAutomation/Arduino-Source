/*  Sandwich Hand Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_SandwichHandDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class SandwichFreeHandMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichFreeHandMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichHand-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichFreeHandMatcher matcher;
        return matcher;
    }
};


class SandwichGrabbingHandMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichGrabbingHandMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichGrab-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichGrabbingHandMatcher matcher;
        return matcher;
    }
};

} // anonymous namespace

std::string SANDWICH_HAND_TYPE_NAMES(SandwichHandType type){
    switch (type)
    {
    case SandwichHandType::FREE:
        return "FREE";
    case SandwichHandType::GRABBING:
        return "GRABBING";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown SandwichHandType");
    }
}

SandwichHandLocator::SandwichHandLocator(HandType hand_type, const ImageFloatBox& box, Color color)
: m_type(hand_type), m_box(box), m_color(color) {}

void SandwichHandLocator::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

std::pair<double, double> SandwichHandLocator::detect(const ImageViewRGB32& frame) const{
    ImageFloatBox entire_screen(0.0, 0.0, 1.0, 1.0);
    std::pair<double, double> location = locate_sandwich_hand(frame, m_box);
    if (location.first >= 0.0){
        return location;
    }else{
        return locate_sandwich_hand(frame, entire_screen);
    }
    
}


std::pair<double, double> SandwichHandLocator::locate_sandwich_hand(const ImageViewRGB32& frame, ImageFloatBox area_to_search) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);

    double min_hand_size = ((m_type == HandType::FREE) ? 5000.0 : 4500.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_hand_size);

    std::pair<double, double> hand_location(-1.0, -1.0);

    ImagePixelBox pixel_box = floatbox_to_pixelbox(frame.width(), frame.height(), area_to_search);
    match_template_by_waterfill(
        extract_box_reference(frame, area_to_search), 
        ((m_type == HandType::FREE) ? SandwichFreeHandMatcher::instance() : SandwichGrabbingHandMatcher::instance()),
        filters,
        {min_size, SIZE_MAX},
        80,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            hand_location = std::make_pair(
                (object.center_of_gravity_x() + pixel_box.min_x) / (double)frame.width(),
                (object.center_of_gravity_y() + pixel_box.min_y) / (double)frame.height()
            );
            return true;
        }
    );

    return hand_location;
}


SandwichHandWatcher::SandwichHandWatcher(
    HandType hand_type,
    const ImageFloatBox& box,
    Color color
): VisualInferenceCallback("SandwichHandWatcher"), m_locator(hand_type, box, color), m_location(-1.0, -1.0) {}

void SandwichHandWatcher::make_overlays(VideoOverlaySet& items) const{
    m_locator.make_overlays(items);
}

bool SandwichHandWatcher::process_frame(const VideoSnapshot& frame){
    m_last_snapshot = frame;
    m_location = m_locator.detect(frame);
    return m_location.first >= 0.0;
}

bool SandwichHandWatcher::recover_sandwich_hand_position(const ImageViewRGB32& frame){
    ImageFloatBox entire_screen(0.0, 0.0, 1.0, 1.0);
    m_location = m_locator.locate_sandwich_hand(frame, entire_screen);
    return m_location.first >= 0.0;
}


}
}
}
