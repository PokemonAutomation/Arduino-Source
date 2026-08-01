/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonSV_Tests_H
#define PokemonAutomation_Tests_PokemonSV_Tests_H

#include <vector>
#include <string>

namespace PokemonAutomation{

class ImageViewRGB32;

int test_pokemonSV_MapDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_PicnicDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_TeraCardFinder(const ImageViewRGB32& image, bool target);

int test_pokemonSV_TerastallizingDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_TeraSilhouetteReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonSV_TeraTypeReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonSV_SandwichRecipeDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_SandwichHandDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_BoxPokemonInfoDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_BoxEggDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_SomethingInBoxSlotDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_BoxPartyEggDetector(const ImageViewRGB32& image, int target);

int test_pokemonSV_OverworldDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_BoxBottomButtonDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_SandwichIngredientsDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_SandwichIngredientReader(const std::string& filepath);

int test_pokemonSV_AdvanceDialogDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_SwapMenuDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_DialogBoxDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_FastTravelDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_MapPokeCenterIconDetector(const ImageViewRGB32& image, int target);

int test_pokemonSV_ESPPressedEmotionDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_MapFlyMenuDetector(const ImageViewRGB32& image, bool target);

int test_pokemonSV_SandwichPlateDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

int test_pokemonSV_RecentlyBattledDetector(const ImageViewRGB32& image, bool target);

}

#endif
