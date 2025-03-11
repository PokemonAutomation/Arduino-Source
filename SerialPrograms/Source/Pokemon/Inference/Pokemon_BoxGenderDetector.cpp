/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "Pokemon_BoxGenderDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

BoxGenderDetector::BoxGenderDetector(const ImageFloatBox& box, double area_ratio_threshold, Color color)
: m_box(box), m_area_ratio_threshold(area_ratio_threshold), m_color(color) {}

void BoxGenderDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

StatsHuntGenderFilter BoxGenderDetector::detect(const ImageViewRGB32& screen) const{
    const auto region = extract_box_reference(screen, m_box);

    // Retain only red pixels from region
    const bool replace_color_within_range = false;
    const ImageRGB32 red_region = filter_rgb32_range(
        region,
        combine_rgb(150, 0, 0), combine_rgb(255, 100, 100), Color(0), replace_color_within_range
    );
    const size_t num_red_pixels = image_stats(red_region).count;

    // Retain only blue pixels from region
    const ImageRGB32 blue_region = filter_rgb32_range(
        region,
        combine_rgb(0, 0, 180), combine_rgb(130, 130, 255), Color(0), replace_color_within_range
    );
    const size_t num_blue_pixels = image_stats(blue_region).count;

    const double threshold = region.width() * region.height() * m_area_ratio_threshold;

    if (PreloadSettings::debug().COLOR_CHECK){
        cout << "num_red_pixels: " << num_red_pixels << ", num_blue_pixels: " << num_blue_pixels
             << ", region " << region.width() << " x " << region.height() << " threshold " << threshold << endl;

        cout << "Save images to ./red_only.png and ./blue_only.png" << endl;
        red_region.save("./red_only.png");
        blue_region.save("./blue_only.png");
    }

    if (num_red_pixels > threshold){
        return Pokemon::StatsHuntGenderFilter::Female;
    }else if (num_blue_pixels > threshold){
        return Pokemon::StatsHuntGenderFilter::Male;
    }
    return Pokemon::StatsHuntGenderFilter::Genderless;
}

}
}
