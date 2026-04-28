/*  Tera Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_TeraTypeReader_H
#define PokemonAutomation_PokemonHome_TeraTypeReader_H

#include <map>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation {
namespace Pokemon {

//  Find all tera type symbols inside the image.
std::multimap<double, std::pair<PokemonTeraType, ImagePixelBox>> find_tera_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image,
    double max_area_ratio
);

// Reads the tera type of a Pokemon.
PokemonTeraType read_pokemon_tera_type(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box
);

}
}

#endif
