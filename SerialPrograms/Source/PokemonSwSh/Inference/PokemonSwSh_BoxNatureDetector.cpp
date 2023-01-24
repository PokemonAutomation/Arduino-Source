/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_BoxNatureDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

BoxNatureDetector::BoxNatureDetector(VideoOverlay& overlay)
    : m_box_atk(overlay, { 0.689, 0.198 + 1 * 0.0515, 0.84, 0.047})
    , m_box_def(overlay, { 0.689, 0.198 + 2 * 0.0515, 0.84, 0.047})
    , m_box_spatk(overlay, { 0.689, 0.198 + 3 * 0.0515, 0.84, 0.047})
    , m_box_spdef(overlay, { 0.689, 0.198 + 4 * 0.0515, 0.84, 0.047})
    , m_box_spd(overlay, { 0.689, 0.198 + 5 * 0.0515, 0.84, 0.047})
{}

NaturePlusMinus BoxNatureDetector::read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box){

    const auto region = extract_box_reference(frame, box);

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

    if (PreloadSettings::debug().COLOR_CHECK) {
        cout << "num_red_pixels: " << num_red_pixels << ", num_blue_pixels: " << num_blue_pixels
            << ", region " << region.width() << " x " << region.height() << " threshold " << threshold << endl;

        cout << "Save images to ./red_only.png and ./blue_only.png" << endl;
        red_region.save("./red_only.png");
        blue_region.save("./blue_only.png");
    }

    if (num_red_pixels > threshold) {
        return NaturePlusMinus::PLUS;
    }
    else if (num_blue_pixels > threshold) {
        return NaturePlusMinus::MINUS;
    }
    return NaturePlusMinus::NEUTRAL;

}
NatureReader::Results BoxNatureDetector::read(Logger& logger, const ImageViewRGB32& frame){
    NatureReader::Results results;
    NaturePlusMinus stats[5];
    int statPlus = -1;
    int statMinus = -1;

    stats[0] = read(logger, frame, m_box_atk);
    stats[1] = read(logger, frame, m_box_def);
    stats[2] = read(logger, frame, m_box_spatk);
    stats[3] = read(logger, frame, m_box_spdef);
    stats[4] = read(logger, frame, m_box_spd);

    for (int i = 0; i < 5; i++) {
        if (stats[i] == NaturePlusMinus::PLUS) {
            statPlus = i;
        }
        else if (stats[i] == NaturePlusMinus::MINUS) {
            statMinus = i;
        }
    }

    results.nature = NatureCheckerValue_helphinder_to_enum(std::make_pair(statPlus, statMinus));
    return results;
}


}
}
}

