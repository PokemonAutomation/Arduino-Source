/*  Moves Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonPokopia_MovesDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



OverworldDetector::OverworldDetector(
    Color color,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_overlay(overlay)
{}

void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, MOVES_LEFT_ARROW_BOX);
    items.add(m_color, MOVES_RIGHT_ARROW_BOX);
}

bool OverworldDetector::detect(const ImageViewRGB32& screen){
    ButtonDetector moves_left_detector(m_color, ButtonType::ButtonDpadLeft, MOVES_LEFT_ARROW_BOX, m_overlay);
    ButtonDetector moves_right_detector(m_color, ButtonType::ButtonDpadRight, MOVES_RIGHT_ARROW_BOX, m_overlay);

    bool found = moves_left_detector.detect(screen) && moves_right_detector.detect(screen);

    return found;
}

PPDetector::PPDetector(
    Color color,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_pp_box({0.827000, 0.772500, 0.137500, 0.075000})
{}

void PPDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_pp_box);
}

bool PPDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 region = extract_box_reference(screen, m_pp_box);
    ImageRGB32 cropped = region.copy();

    const size_t box_w = cropped.width();
    const size_t box_h = cropped.height();

    // Apply mask to region to isolate the curved PP bar
    static const ImageRGB32 mask_template(RESOURCE_PATH() + "PokemonPokopia/PPDetector-Mask.png");
    ImageRGB32 mask = mask_template.scale_to(box_w, box_h);
    PackedBinaryMatrix opaque = compress_rgb32_to_binary_range(mask, 0xff000000, 0xffffffff);
    filter_by_mask(opaque, cropped, Color(0, 0, 0, 0), true);

    // Apply filters to get percentage of bar filled and the color of the bar
    std::vector<FilterRgb32Range> filters = {
        {Color(0), false, 0xff000000, 0xffffffff}, // fully opaque
        {Color(0), false, 0xfff0af00, 0xfffff5b9}, // yellow: RGB(240, 175, 0), RGB(255, 245, 185)
        {Color(0), false, 0xff6496e6, 0xffc8d7ff}, // blue: RGB(100, 150, 230), RGB(200, 215, 255)
    };
    std::vector<std::pair<ImageRGB32, size_t>> filter_out = filter_rgb32_range(cropped, filters);
    m_yellow_ratio = filter_out[0].second > 0 ? (double)filter_out[1].second / filter_out[0].second : 0.0;
    m_blue_ratio   = filter_out[0].second > 0 ? (double)filter_out[2].second / filter_out[0].second : 0.0;

    if (m_yellow_ratio > 0 || m_blue_ratio > 0){
        m_pp_percent = std::max(m_yellow_ratio, m_blue_ratio);
        m_powered_up = m_yellow_ratio > 0.01 && m_blue_ratio < 0.01;
        return true;
    }
    return false;
}


}
}
}
