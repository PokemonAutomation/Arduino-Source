/*  Sandwich Hand Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV_SandwichHandDetector.h"

#include <iostream>
using std::cout;
using std::endl;

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

class SandwichFreeHandMatcher2 : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichFreeHandMatcher2() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichHand-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichFreeHandMatcher2 matcher;
        return matcher;
    }
};


class SandwichGrabbingHandMatcher2 : public ImageMatch::WaterfillTemplateMatcher{
public:
    SandwichGrabbingHandMatcher2() : WaterfillTemplateMatcher(
        "PokemonSV/Picnic/SandwichGrab-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static SandwichGrabbingHandMatcher2 matcher;
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
    return locate_sandwich_hand(frame, m_box, false);
}


std::pair<double, double> SandwichHandLocator::locate_sandwich_hand(
    const ImageViewRGB32& frame, ImageFloatBox area_to_search, bool check_outline) const
{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(150, 150, 150), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);

    double min_hand_size = ((m_type == HandType::FREE) ? 5000.0 : 4500.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_hand_size);

    std::pair<double, double> hand_location(-1.0, -1.0);

    ImagePixelBox pixel_box = floatbox_to_pixelbox(frame.width(), frame.height(), area_to_search);
    
    const std::vector<std::pair<uint32_t, uint32_t>> black_filters = {
        // {combine_rgb(0, 0, 0), combine_rgb(80, 80, 80)}
        {combine_rgb(0, 0, 0), combine_rgb(150, 150, 150)}
    };

    match_template_by_waterfill(
        extract_box_reference(frame, area_to_search), 
        ((m_type == HandType::FREE) ? SandwichFreeHandMatcher::instance() : SandwichGrabbingHandMatcher::instance()),
        filters,
        {min_size, SIZE_MAX},
        60,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            if (!check_outline){
                hand_location = std::make_pair(
                    (object.center_of_gravity_x() + pixel_box.min_x) / (double)frame.width(),
                    (object.center_of_gravity_y() + pixel_box.min_y) / (double)frame.height()
                );
                return true;
            }
            else {
                /* 
                - this checks that the matched waterfill object contains the black outline
                - for matching waterfill objects, apply a mask filter to retain all black components
                - if this is a true positive, waterfill_sub_binary_image should be a black outline of the hand. 
                - if false positive, it will be a pure white image
                - then apply the waterfill+template match algorithm on the resulting waterfill_sub_binary_image
                - this helps to remove false positives (e.g. lettuce)
                */
                ImageRGB32 waterfill_sub_binary_image = extract_box_reference(extract_box_reference(frame, area_to_search), object).copy();
                auto matrices = compress_rgb32_to_binary_range(waterfill_sub_binary_image, black_filters);
                filter_by_mask(matrices[0], waterfill_sub_binary_image, Color(COLOR_WHITE), true);

                if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
                    dump_debug_image(
                        global_logger_command_line(), 
                        "CommonFramework/SandwichHandDetector", 
                        "waterfill_sub_binary_image", 
                        waterfill_sub_binary_image);
                }
                
                match_template_by_waterfill(
                    waterfill_sub_binary_image, 
                    ((m_type == HandType::FREE) ? SandwichFreeHandMatcher2::instance() : SandwichGrabbingHandMatcher2::instance()),
                    filters,
                    {min_size, SIZE_MAX},
                    60,
                    [&](Kernels::Waterfill::WaterfillObject& object2) -> bool {
                        hand_location = std::make_pair(
                            (object.center_of_gravity_x() + pixel_box.min_x) / (double)frame.width(),
                            (object.center_of_gravity_y() + pixel_box.min_y) / (double)frame.height()
                        );
                        return true;
                    }
                );
                return false;
            }

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

std::pair<double, double> SandwichHandWatcher::search_entire_screen_for_sandwich_hand(const ImageViewRGB32& frame) const{
    ImageFloatBox entire_screen(0.0, 0.0, 1.0, 1.0);
    return m_locator.locate_sandwich_hand(frame, entire_screen, true);
}


}
}
}
