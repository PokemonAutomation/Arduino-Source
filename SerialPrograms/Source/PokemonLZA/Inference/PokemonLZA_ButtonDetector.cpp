/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLZA_ButtonDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for buttons
    // - min_width: candidate image min width if video stream is 4k
    // - min_height: candidate image min height if video stream is 4k
    ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd);
    static const ButtonMatcher& A(){
        static ButtonMatcher matcher(ButtonType::ButtonA, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& B(){
        static ButtonMatcher matcher(ButtonType::ButtonB, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& X(){
        static ButtonMatcher matcher(ButtonType::ButtonX, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& Y(){
        static ButtonMatcher matcher(ButtonType::ButtonY, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& L(){
        static ButtonMatcher matcher(ButtonType::ButtonL, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& R(){
        static ButtonMatcher matcher(ButtonType::ButtonR, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& Plus(){
        static ButtonMatcher matcher(ButtonType::ButtonPlus, 30, 30, 80);
        return matcher;
    }
    static const ButtonMatcher& Minus(){
        static ButtonMatcher matcher(ButtonType::ButtonMinus, 30, 30, 80);
        return matcher;
    }
    static const ButtonMatcher& Right(){
        static ButtonMatcher matcher(ButtonType::ButtonRight, 50, 50, 70);
        return matcher;
    }
    static const ButtonMatcher& RightStickUpDown(){
        static ButtonMatcher matcher(ButtonType::RightStickUpDown, 20, 20, 150);
        return matcher;
    }
    static const ButtonMatcher& DpadUp(){
        static ButtonMatcher matcher(ButtonType::ButtonDpadUp, 30, 30, 80);
        return matcher;
    }
    static const ButtonMatcher& DpadDown(){
        static ButtonMatcher matcher(ButtonType::ButtonDpadDown, 30, 30, 80);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 3840;
        size_t min_height = m_min_height * input_resolution.height / 2160;
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
    case ButtonType::ButtonA:
        return "PokemonLZA/Buttons/ButtonA.png";
    case ButtonType::ButtonB:
        return "PokemonLZA/Buttons/ButtonB.png";
    case ButtonType::ButtonX:
        return "PokemonLZA/Buttons/ButtonX.png";
    case ButtonType::ButtonY:
        return "PokemonLZA/Buttons/ButtonY.png";
    case ButtonType::ButtonL:
        return "PokemonLZA/Buttons/ButtonL.png";
    case ButtonType::ButtonR:
        return "PokemonLZA/Buttons/ButtonR.png";
    case ButtonType::ButtonPlus:
        return "PokemonLZA/Buttons/ButtonPlus.png";
    case ButtonType::ButtonMinus:
        return "PokemonLZA/Buttons/ButtonMinus.png";
    case ButtonType::ButtonRight:
        return "PokemonLZA/Buttons/ButtonRight.png";
    case ButtonType::RightStickUpDown:
        return "PokemonLZA/Buttons/RightStickUpDown.png";
    case ButtonType::ButtonDpadUp:
        return "PokemonLZA/Buttons/ButtonDpadUp.png";
    case ButtonType::ButtonDpadDown:
        return "PokemonLZA/Buttons/ButtonDpadDown.png";
    default:
        return "";
    }
}

const char* button_name(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return "ButtonA";
    case ButtonType::ButtonB:
        return "ButtonB";
    case ButtonType::ButtonX:
        return "ButtonX";
    case ButtonType::ButtonY:
        return "ButtonY";
    case ButtonType::ButtonL:
        return "ButtonL";
    case ButtonType::ButtonR:
        return "ButtonR";
    case ButtonType::ButtonPlus:
        return "Button+";
    case ButtonType::ButtonMinus:
        return "Button-";
    case ButtonType::ButtonRight:
        return "ButtonR";
    case ButtonType::RightStickUpDown:
        return "RightStickUpDown";
    case ButtonType::ButtonDpadUp:
        return "ButtonDpadUp";
    case ButtonType::ButtonDpadDown:
        return "ButtonDpadDown";
    default:
        return "";
    }
}

const ButtonMatcher& get_button_matcher(ButtonType type){
    switch (type){
    case ButtonType::ButtonA:
        return ButtonMatcher::A();
    case ButtonType::ButtonB:
        return ButtonMatcher::B();
    case ButtonType::ButtonX:
        return ButtonMatcher::X();
    case ButtonType::ButtonY:
        return ButtonMatcher::Y();
    case ButtonType::ButtonL:
        return ButtonMatcher::L();
    case ButtonType::ButtonR:
        return ButtonMatcher::R();
    case ButtonType::ButtonPlus:
        return ButtonMatcher::Plus();
    case ButtonType::ButtonMinus:
        return ButtonMatcher::Minus();
    case ButtonType::ButtonRight:
        return ButtonMatcher::Right();
    case ButtonType::RightStickUpDown:
        return ButtonMatcher::RightStickUpDown();
    case ButtonType::ButtonDpadUp:
        return ButtonMatcher::DpadUp();
    case ButtonType::ButtonDpadDown:
        return ButtonMatcher::DpadDown();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for ButtonType");
    }
}




ButtonMatcher::ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(
        template_path(type),
        Color(0xff808008),
        Color(0xffffffff),
        100
    )
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

    double screen_rel_size = (screen.height() / 2160.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_3840p = 1000.0;
//    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_3840p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
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




















}
}
}
