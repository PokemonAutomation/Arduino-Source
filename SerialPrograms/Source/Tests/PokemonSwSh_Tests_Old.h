/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/
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

int test_pokemonSwSh_DialogTriangleDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_RetrieveEggArrowFinder(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_StoragePokemonMenuArrowFinder(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_CheckNurseryArrowFinder(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_YCommIconDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_RotomPhoneMenuArrowFinder(const ImageViewRGB32& image, int target);

int test_pokemonSwSh_BlackDialogBoxDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_BoxShinySymbolDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSwSh_BoxGenderDetector(const ImageViewRGB32& image, int target);

int test_pokemonSwSh_SelectionArrowFinder(const ImageViewRGB32& image, int target);

}

#endif
