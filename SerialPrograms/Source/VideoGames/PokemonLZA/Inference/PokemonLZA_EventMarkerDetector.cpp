/*  Event Marker Detector
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
#include "PokemonLZA_EventMarkerDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

EventMarkerDetector::EventMarkerDetector(Color color)
    : m_color(color)
    // Camera can put the event marker against the top edge
    , m_search_box(0.02, 0.0, 0.96, 0.73)
{}

void EventMarkerDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_search_box);
}

bool EventMarkerDetector::detect(const ImageViewRGB32& screen){
    m_marker_box = {};
    const ImagePixelBox search = floatbox_to_pixelbox(screen, m_search_box);
    ImageRGB32 crop = extract_box_reference(screen, search).copy();
    // Hide the minimap, so we don't accidentally match the marker on it
    const size_t minimap_right = size_t(screen.width() * 0.18);
    const size_t minimap_bottom = size_t(screen.height() * 0.30);
    for (size_t y = search.min_y; y < std::min(search.max_y, minimap_bottom); y++){
        for (size_t x = search.min_x; x < std::min(search.max_x, minimap_right); x++){
            crop.pixel(x - search.min_x, y - search.min_y) = 0xff000000;
        }
    }
    PackedBinaryMatrix mask = compress_rgb32_to_binary_range(crop, 200, 255, 140, 255, 0, 140);
    const double scale = screen.height() / 1080.0;
    const auto objects = Kernels::Waterfill::find_objects_inplace(mask, size_t(120 * scale * scale));
    bool found = false;
    for (const auto& object : objects){
        const double width = static_cast<double>(object.width());
        const double height = static_cast<double>(object.height());
        const double aspect = width / height;
        if (width < 15 * scale || width > 100 * scale || height < 20 * scale || height > 120 * scale ||
            aspect < 0.5 || aspect > 1.1 || object.area < width * height * 0.45){
            continue;
        }
        const ImagePixelBox badge(
            search.min_x + object.min_x, search.min_y + object.min_y,
            search.min_x + object.max_x, search.min_y + object.max_y
        );
        // Check the shape of !, ignoring the white border
        const auto white_fraction = [&](double x0, double y0, double x1, double y1){
            const ImagePixelBox box(
                badge.min_x + size_t(width * x0), badge.min_y + size_t(height * y0),
                badge.min_x + size_t(width * x1), badge.min_y + size_t(height * y1)
            );
            size_t white = 0;
            for (size_t y = box.min_y; y < box.max_y; y++){
                for (size_t x = box.min_x; x < box.max_x; x++){
                    const uint32_t pixel = screen.pixel(x, y);
                    if (((pixel >> 16) & 0xff) >= 200 && ((pixel >> 8) & 0xff) >= 200 && (pixel & 0xff) >= 180){
                        white++;
                    }
                }
            }
            return white / static_cast<double>(box.area());
        };
        // Sample the shape of !, ignoring the white border
        if (white_fraction(0.4, 0.2, 0.6, 0.55) < 0.3 ||
            white_fraction(0.35, 0.65, 0.65, 0.85) < 0.1 ||
            white_fraction(0.35, 0.55, 0.65, 0.65) > 0.65){
            continue;
        }
        // Off-center markers omit the green flag while the yellow ! badge is shared.
        if (found){
            m_marker_box = {};
            return false;
        }
        m_marker_box = pixelbox_to_floatbox(screen, badge);
        found = true;
    }
    return found;
}

}
}
}
