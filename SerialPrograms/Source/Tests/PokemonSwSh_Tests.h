/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonSwSh_Tests_H
#define PokemonAutomation_Tests_PokemonSwSh_Tests_H

#include <vector>
#include <string>

namespace PokemonAutomation{

class ImageViewRGB32;

int test_pokemonSwSh_YCommMenuDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_MaxLair_BattleMenuDetector(const ImageViewRGB32& image, bool target);


}

#endif
