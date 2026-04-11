/*  Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_TypeReader_H
#define PokemonAutomation_Pokemon_TypeReader_H

#include <map>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation {
namespace Pokemon {

// The style of type symbols.
// GEN8: SWSH, Needs testing: LGPE
// GEN9: SV, LZA, HOME(Currently), Needs testing: Champions, LA, BDSP
enum class PokemonTypeGeneration{
    GEN8,
	GEN9,
};

//  Find all type symbols inside the image.
std::multimap<double, std::pair<PokemonType, ImagePixelBox>> find_type_symbols(
    const ImageViewPlanar32& original_screen,
    const ImageViewRGB32& image, 
    double max_area_ratio,
    PokemonTypeGeneration generation
);

// Reads the types of a Pokemon. Second type will be PokemonType::NONE if the Pokemon is single-type
std::pair<PokemonType, PokemonType> read_pokemon_types(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box, 
    PokemonTypeGeneration generation
);
    
}
}
#endif