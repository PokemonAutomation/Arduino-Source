/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonHome_ButtonDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for buttons
    // - min_width: candidate image min width if video stream is 4k
    // - min_height: candidate image min height if video stream is 4k
    ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd);
    
    static const ButtonMatcher& B(){
        static ButtonMatcher matcher(ButtonType::ButtonB, 25, 25, 80);
        return matcher;
    }
    static const ButtonMatcher& Plus(){
        static ButtonMatcher matcher(ButtonType::ButtonPlus, 25, 25, 80);
        return matcher;
    }
    

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 1920;
        size_t min_height = m_min_height * input_resolution.height / 1080;
//        cout << "???? check_image() ???? min size " << min_width << " x " << min_height
//             << " got " << image.width() << " x " << image.height() << endl;
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};







const char* template_path(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return "PokemonHome/Buttons/ButtonB-Template.png";
    case ButtonType::ButtonPlus:
        return "PokemonHome/Buttons/ButtonPlus-Template.png";
    default:
        return "";
    }
}

const char* button_name(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return "ButtonB";
    case ButtonType::ButtonPlus:
        return "ButtonPlus";
    default:
        return "";
    }
}

const ButtonMatcher& get_button_matcher(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return ButtonMatcher::B();
    case ButtonType::ButtonPlus:
        return ButtonMatcher::Plus();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for ButtonType");
    }
}



ButtonMatcher::ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(template_path(type), COLOR_BLACK, COLOR_WHITE, 100)
    , m_min_width(min_width)
    , m_min_height(min_height)
    , m_max_rmsd(max_rmsd)
{}



ButtonDetector::ButtonDetector(
    Color color,
    ButtonType button_type,
    const ImageFloatBox& box,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_matcher(get_button_matcher(button_type))
    , m_box(box)
    , m_overlay(overlay)
{

}
void ButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool ButtonDetector::detect(const ImageViewRGB32& screen){

    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 500.0;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff000000, 0xffe0e0e0},
        {0xff000000, 0xffd0d0d0},
        {0xff000000, 0xffc0c0c0},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        FILTERS,
        {min_area, SIZE_MAX},
        m_matcher.m_max_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}


BoxViewDetector::BoxViewDetector(VideoOverlay* overlay) : m_button_plus_detector(COLOR_BLACK, ButtonType::ButtonPlus, {0.100, 0.956, 0.107, 0.041}, overlay){}

void BoxViewDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_plus_detector.make_overlays(items);
}

bool BoxViewDetector::detect(const ImageViewRGB32& screen){
    return m_button_plus_detector.detect(screen);
}


SummaryScreenDetector::SummaryScreenDetector(VideoOverlay* overlay) : m_button_B_detector(COLOR_BLACK, ButtonType::ButtonB, {0.100, 0.956, 0.107, 0.041}, overlay){}

void SummaryScreenDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_B_detector.make_overlays(items);
}

bool SummaryScreenDetector::detect(const ImageViewRGB32& screen){
    return m_button_B_detector.detect(screen);
}








}
}
}
