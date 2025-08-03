/*  Type Symbol Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TypeSymbolFinder_H
#define PokemonAutomation_PokemonSwSh_TypeSymbolFinder_H

#include <map>
#include "CommonFramework/ImageTypes/ImageViewPlanar32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_Types.h"
//#include "PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


//  Find all type symbols inside the image.
std::multimap<double, std::pair<PokemonType, ImagePixelBox>> find_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image, double max_area_ratio
);



void test_find_type_symbols(
    ProgramEnvironment& env,
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    const ImageViewRGB32& screen, double max_area_ratio = 0.20
);


}
}
}
#endif
