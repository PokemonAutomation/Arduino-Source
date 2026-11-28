/*  Booth Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonLZA_BoothDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

BoothDetector::BoothDetector(Color color)
    : m_color(color)
    , m_search_box(0.18, 0.12, 0.78, 0.55)
{}

void BoothDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_search_box);
}

bool BoothDetector::detect(const ImageViewRGB32& screen){
    m_roof_box = {};
    const ImagePixelBox search = floatbox_to_pixelbox(screen, m_search_box);
    const ImageViewRGB32 crop = extract_box_reference(screen, search);
    // Keep the bright cyan strip, excluding beige walls and the green zone gate.
    ImageRGB32 cyan = crop.copy();
    for (size_t y = 0; y < cyan.height(); y++){
        for (size_t x = 0; x < cyan.width(); x++){
            const uint32_t pixel = cyan.pixel(x, y);
            const uint32_t r = (pixel >> 16) & 0xff;
            const uint32_t g = (pixel >> 8) & 0xff;
            const uint32_t b = pixel & 0xff;
            const bool is_cyan = g >= 180 && b >= 180 && r + 25 <= std::min(g, b) &&
                std::max(g, b) - std::min(g, b) <= 60;
            cyan.pixel(x, y) = is_cyan ? 0xffffffff : 0xff000000;
        }
    }
    PackedBinaryMatrix mask = compress_rgb32_to_binary_range(cyan, 0xffffffff, 0xffffffff);
    const double scale = screen.height() / 1080.0;
    const auto objects = Kernels::Waterfill::find_objects_inplace(mask, size_t(100 * scale * scale));
    bool found = false;
    for (const auto& object : objects){
        const double width = static_cast<double>(object.width());
        const double height = static_cast<double>(object.height());
        // Try to find the long cyan strip on the top the booth
        if (width < 70 * scale || width > 600 * scale || height < 2 * scale ||
            width / height < 15 || width / height > 120 || object.area < width * height * 0.3){
            continue;
        }
        const ImagePixelBox roof(
            search.min_x + object.min_x, search.min_y + object.min_y,
            search.min_x + object.max_x, search.min_y + object.max_y
        );
        // Require the gray roof above the strip to reject cyan battle boundaries.
        if (roof.min_y < width * 0.1){
            continue;
        }
        const ImagePixelBox roof_top(
            roof.min_x + size_t(width * 0.15), roof.min_y - size_t(width * 0.1),
            roof.max_x - size_t(width * 0.15), roof.min_y - size_t(width * 0.025)
        );
        size_t gray = 0;
        for (size_t y = roof_top.min_y; y < roof_top.max_y; y++){
            for (size_t x = roof_top.min_x; x < roof_top.max_x; x++){
                const uint32_t pixel = screen.pixel(x, y);
                const uint32_t r = (pixel >> 16) & 0xff;
                const uint32_t g = (pixel >> 8) & 0xff;
                const uint32_t b = pixel & 0xff;
                if (r >= 90 && r <= 235 && g >= 100 && b >= 100 &&
                    std::max(g, b) - std::min(g, b) <= 40 &&
                    std::max(r, g) - std::min(r, g) <= 60){
                    gray++;
                }
            }
        }
        if (gray < roof_top.area() * 0.5){
            continue;
        }
        // The cyan strip must sit above a substantial dark wall, not sky or HUD.
        const ImagePixelBox wall(
            roof.min_x + size_t(width * 0.15), roof.max_y + size_t(width * 0.08),
            roof.max_x - size_t(width * 0.15), roof.max_y + size_t(width * 0.5)
        );
        if (wall.max_y > screen.height()){
            continue;
        }
        size_t dark = 0;
        for (size_t y = wall.min_y; y < wall.max_y; y++){
            for (size_t x = wall.min_x; x < wall.max_x; x++){
                const uint32_t pixel = screen.pixel(x, y);
                const uint32_t r = (pixel >> 16) & 0xff;
                const uint32_t g = (pixel >> 8) & 0xff;
                const uint32_t b = pixel & 0xff;
                if (r <= 110 && g <= 145 && b <= 145){
                    dark++;
                }
            }
        }
        if (dark < wall.area() * 0.4){
            continue;
        }
        // Ambiguous views must not steer toward an arbitrary candidate.
        if (found){
            m_roof_box = {};
            return false;
        }
        m_roof_box = pixelbox_to_floatbox(screen, roof);
        found = true;
    }
    return found;
}

}
}
}
