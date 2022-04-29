/*  Item Compatibility Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonLA_ItemCompatibilityDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ItemCompatibility detect_item_compatibility(const QImage& screen){
    // The Compatible/Incompatible text region of the lead pokemon on the screen.
    ImageFloatBox box(0.838, 0.1815, 0.090, 0.024);

    QImage region = extract_box_copy(screen, box);

    // Replacing white background with zero-alpha color so that they won't be counted in
    // the following image_stats()
    filter_rgb32_range(region, 0xffa0a0a0, 0xffffffff, Color(0), true);

    ImageStats stats = image_stats(region);
    // std::cout << "Compability color " << stats.average.r << " " << stats.average.g << " " << stats.average.b << std::endl;
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
