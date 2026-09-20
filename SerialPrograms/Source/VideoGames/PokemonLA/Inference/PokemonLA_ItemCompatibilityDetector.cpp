/*  Item Compatibility Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLA_ItemCompatibilityDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ItemCompatibility detect_item_compatibility(const ImageViewRGB32& screen){
    // The Compatible/Incompatible text region of the lead pokemon on the screen.
    const ImageFloatBox box(0.838, 0.1815, 0.090, 0.024);

    // Replacing white background with zero-alpha color so that they won't be counted in
    // the following image_stats()
    // The white background is defined as the color between 0xffa0a0a0 and 0xffffffff.
    const bool replace_color_within_range = true;
    ImageRGB32 region = filter_rgb32_range(
        extract_box_reference(screen, box),
        0xffa0a0a0, 0xffffffff, Color(0), replace_color_within_range
    );

    ImageStats stats = image_stats(region);
    // std::cout << "Compatibility color " << stats.average.r << " " << stats.average.g << " " << stats.average.b << std::endl;
    if (stats.average.r > stats.average.b + 50.0){
        return ItemCompatibility::INCOMPATIBLE;
    }
    if (stats.average.b > stats.average.r + 50.0){
        return ItemCompatibility::COMPATIBLE;
    }

    return ItemCompatibility::NONE;
}


}
}
}
