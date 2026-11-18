/*  BDSP Button Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include <algorithm>
#include <array>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonBDSP_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
namespace{

class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    ButtonMatcher(const char* path)
        : WaterfillTemplateMatcher(path, Color(0xff303030), Color(0xffa0a0a0), 100)
    {
        // First crop to the gray circle, excluding any solid white background.
        // Then extract the interior letter, as in PLZA's button waterfills.
        auto matrix = compress_rgb32_to_binary_range(image_template(), 0xffc0c0c0, 0xffffffff);
        const auto objects = Kernels::Waterfill::find_objects_inplace(matrix, 20);
        const Kernels::Waterfill::WaterfillObject* letter = nullptr;
        for (const auto& object : objects){
            if (object.min_x == 0 || object.min_y == 0 ||
                object.max_x == image_template().width() || object.max_y == image_template().height()){
                continue;
            }
            if (letter == nullptr || object.area > letter->area){
                letter = &object;
            }
        }
        if (letter == nullptr){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Button template has no interior letter.", path);
        }
        m_area_ratio = letter->area_ratio();
        m_matcher.reset(new ImageMatch::ExactImageMatcher(extract_box_reference(image_template(), *letter).copy()));
        m_area_ratio_lower = 0.70;
        m_area_ratio_upper = 1.40;
    }
};

const ButtonMatcher& button_matcher(ButtonType type){
    switch (type){
    case ButtonType::A:{
        static const ButtonMatcher matcher("PokemonBDSP/a-button.png");
        return matcher;
    }
    case ButtonType::B:{
        static const ButtonMatcher matcher("PokemonBDSP/b-button.png");
        return matcher;
    }
    case ButtonType::X:{
        static const ButtonMatcher matcher("PokemonBDSP/x-button.png");
        return matcher;
    }
    case ButtonType::Y:{
        static const ButtonMatcher matcher("PokemonBDSP/y-button.png");
        return matcher;
    }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown BDSP button type.");
}

bool has_gray_surround(const ImageViewRGB32& image, const Kernels::Waterfill::WaterfillObject& object){
    const double cx = (object.min_x + object.max_x) * 0.5;
    const double cy = (object.min_y + object.max_y) * 0.5;
    const double height = (double)object.height();
    static const std::array<std::pair<double, double>, 8> offsets{{
        {-0.62, 0}, {0.62, 0}, {0, -0.65}, {0, 0.65},
        {-0.45, -0.45}, {0.45, -0.45}, {-0.45, 0.45}, {0.45, 0.45},
    }};
    size_t gray = 0;
    for (const auto& [dx, dy] : offsets){
        const int x = (int)(cx + dx * height);
        const int y = (int)(cy + dy * height);
        if (x < 0 || y < 0 || (size_t)x >= image.width() || (size_t)y >= image.height()){
            return false;
        }
        const uint32_t pixel = image.pixel(x, y);
        const int r = (pixel >> 16) & 255, g = (pixel >> 8) & 255, b = pixel & 255;
        gray += std::min({r, g, b}) >= 20 && std::max({r, g, b}) <= 180 &&
            std::max({r, g, b}) - std::min({r, g, b}) < 40;
    }
    return gray >= 6;
}

}

bool ButtonDetector::detect(const ImageViewRGB32& screen, ImageFloatBox* matched_box) const{
    if (!screen){
        return false;
    }
    const auto region = extract_box_reference(screen, m_box);
    const double scale = screen.height() / 1080.0;
    static const std::vector<std::pair<uint32_t, uint32_t>> filters{
        {0xffc0c0c0, 0xffffffff}, {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff}, {0xffd0d0d0, 0xffffffff}, {0xffe0e0e0, 0xffffffff},
    };
    bool found = false;
    match_template_by_waterfill(screen.size(), region, button_matcher(m_type), filters,
        {std::max(size_t(3), (size_t)(60 * scale * scale)), (size_t)(700 * scale * scale)}, 70.0,
        [&](Kernels::Waterfill::WaterfillObject& object){
            if (object.height() < 16 * scale || object.height() > 36 * scale ||
                !has_gray_surround(region, object)){
                return false;
            }
            if (matched_box){
                *matched_box = translate_to_parent(screen, m_box, object);
            }
            found = true;
            return true;
        });
    return found;
}

}
}
}
