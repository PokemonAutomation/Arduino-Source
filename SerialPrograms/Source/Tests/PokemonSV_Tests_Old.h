/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Tests_PokemonSV_Tests_H
#define PokemonAutomation_Tests_PokemonSV_Tests_H

namespace PokemonAutomation{

class ImageViewRGB32;

int test_pokemonSV_AdvanceDialogDetector(const ImageViewRGB32& image, bool target);
int test_pokemonSV_SwapMenuDetector(const ImageViewRGB32& image, bool target);

}

#endif
