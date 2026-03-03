/*  PokemonFRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 */


#ifndef PokemonAutomation_Tests_PokemonFRLG_Tests_H
#define PokemonAutomation_Tests_PokemonFRLG_Tests_H

#include <vector>
#include <string>

namespace PokemonAutomation{

class ImageViewRGB32;


int test_pokemonFRLG_AdvanceWhiteDialogDetector(const ImageViewRGB32& image, bool target);

int test_pokemonFRLG_ShinySymbolDetector(const ImageViewRGB32& image, bool target);

}

#endif
