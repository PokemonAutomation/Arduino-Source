/*  Map Zoom Level Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonLA_MapZoomLevelReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

int read_map_zoom_level(const ImageViewRGB32& screen){
    
    // The three locations of the yellow disk on the zoom gauge, from left to right.
    // Left most is Hisui map, zoom level 0. Right most is local view, zoom level 2.
    const ImageFloatBox boxes[3] = {
        {0.780, 0.085, 0.008, 0.014},
        {0.795, 0.082, 0.010, 0.019},
        {0.807, 0.081, 0.014, 0.022},
    };

    double max_yellow = 0;
    int max_yellow_index = -1;

    for (int i = 0; i < 3; i++){

        // Replacing non-yellow color with zero-alpha color so that they won't be counted in
        // the following image_stats()
        const bool replace_background = true;
        size_t pixels_filtered;
        ImageRGB32 region = filter_rgb32_range(
            pixels_filtered,
            extract_box_reference(screen, boxes[i]),
            combine_rgb(0, 0, 0), combine_rgb(200, 200, 255), Color(0), replace_background
        );
        if (pixels_filtered == (size_t)region.width() * (size_t)region.height()){
            // All pixels are filtered out, so no yellow color. In this case, this is defenitiely not the location
            // of the yellow disk:
            // std::cout << "No yellow disk at " << i << std::endl;
            continue;
        }

        const auto stats = image_stats(region);

        if (std::isnan(stats.average.r) == false && std::isnan(stats.average.g) == false){
            const double yellow = (stats.average.r + stats.average.g) / 2.0;
            if (yellow > max_yellow){
                max_yellow_index = i;
                max_yellow = yellow;
            }
        }
        // std::cout << "Compatibility color " << stats.average << " " << stats.stddev << std::endl;
    }

    return max_yellow_index;
}


}
}
}
