/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonPokopia_ButtonDetector.h"



namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for buttons
    // - min_width: candidate image min width if video stream is 1080p
    // - min_height: candidate image min height if video stream is 1080p
    ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd);
    static const ButtonMatcher& A(){
        static ButtonMatcher matcher(ButtonType::ButtonA, 40, 40, 70);
        return matcher;
    }
    static const ButtonMatcher& DpadLeft(){
        static ButtonMatcher matcher(ButtonType::ButtonDpadLeft, 30, 30, 120);
        return matcher;
    }
    static const ButtonMatcher& DpadRight(){
        static ButtonMatcher matcher(ButtonType::ButtonDpadRight, 30, 30, 120);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 1920;
        size_t min_height = m_min_height * input_resolution.height / 1080;

        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            const double stddev_sum = image_stats(image).stddev.sum();
            std::cout << "???? ButtonMatcher::check_image() ???? min size " << min_width << " x " << min_height
                 << " got " << image.width() << " x " << image.height() << " stddev_sum: " << stddev_sum << std::endl;
        }
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


const char* template_path(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return "PokemonPokopia/ButtonA.png";
    case ButtonType::ButtonDpadLeft:
        return "PokemonPokopia/ButtonDpadLeft.png";
    case ButtonType::ButtonDpadRight:
        return "PokemonPokopia/ButtonDpadRight.png";
    default:
        return "";
    }
}

const ButtonMatcher& get_button_matcher(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return ButtonMatcher::A();
    case ButtonType::ButtonDpadLeft:
        return ButtonMatcher::DpadLeft();
    case ButtonType::ButtonDpadRight:
        return ButtonMatcher::DpadRight();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for ButtonType");
    }
}

ButtonMatcher::ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(template_path(type), Color(0xffc0c0c0), Color(0xffffffff), 500)
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
    : m_button_type(button_type)
    , m_color(color)
    , m_matcher(get_button_matcher(button_type))
    , m_box(box)
    , m_overlay(overlay)
{

}
void ButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool ButtonDetector::detect(const ImageViewRGB32& screen){

    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 500;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc0c0c0, 0xffffffff} // RGB(192, 192, 192), RGB(255, 255, 255)
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        FILTERS,
        {min_area, SIZE_MAX},
        m_matcher.m_max_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
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



}
}
}
