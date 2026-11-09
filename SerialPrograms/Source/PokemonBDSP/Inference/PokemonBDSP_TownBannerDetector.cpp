/*  Town Banner Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonBDSP_TownBannerDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

TownBannerDetector::TownBannerDetector(Color color)
    : m_color(color)
    , m_white{{
        {0.026, 0.026, 0.400, 0.006},
        {0.026, 0.097, 0.400, 0.006},
        {0.026, 0.040, 0.010, 0.045},
        {0.424, 0.040, 0.010, 0.045},
    }}
    , m_border(0.032, 0.113, 0.390, 0.003)
{}

void TownBannerDetector::make_overlays(VideoOverlaySet& items) const{
    for (const auto& box : m_white){
        items.add(m_color, box);
    }
    items.add(m_color, m_border);
}

bool TownBannerDetector::detect(const ImageViewRGB32& screen){
    if (screen.width() < 640 || screen.height() < 360){
        return false;
    }
    // Grayscale-equivalent mean brightness accepts tinted white from
    // different capture cards. Do not require equal red/green/blue values.
    // Sample the padding, avoiding both the name and the rounded corners.
    double darkest_padding = 255;
    double brightest_padding = 0;
    for (const auto& box : m_white){
        const ImageStats stats = image_stats(extract_box_reference(screen, box));
        const double brightness = stats.average.sum() / 3;
        if (brightness < 160 || stats.stddev.sum() > 30){
            return false;
        }
        darkest_padding = std::min(darkest_padding, brightness);
        brightest_padding = std::max(brightest_padding, brightness);
    }
    if (brightest_padding - darkest_padding > 20){
        return false;
    }
    // The dark bottom outline distinguishes the banner from white fades,
    // pale scenery, and other bright panels. No text/template is required.
    const ImageStats border = image_stats(extract_box_reference(screen, m_border));
    return darkest_padding - border.average.sum() / 3 >= 60;
}

}
}
}
