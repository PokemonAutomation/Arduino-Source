/*  Type Symbol Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TypeSymbolFinder_H
#define PokemonAutomation_PokemonSwSh_TypeSymbolFinder_H

#include <map>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonSwSh{


//  Find all type symbols inside the image.
std::multimap<double, std::pair<PokemonType, ImagePixelBox>> find_symbols(
    const ImageViewRGB32& image, double max_area_ratio
);



void test_find_symbols(
    ProgramEnvironment& env,
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    const ImageViewRGB32& screen, double max_area_ratio = 0.20
);


}
}
}
#endif
