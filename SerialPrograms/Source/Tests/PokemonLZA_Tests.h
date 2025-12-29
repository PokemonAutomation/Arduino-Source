/*  PokemonLZA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


 #ifndef PokemonAutomation_Tests_PokemonLZA_Tests_H
 #define PokemonAutomation_Tests_PokemonLZA_Tests_H
 
 #include <vector>
 #include <string>
 
 #include "CommonFramework/AudioPipeline/AudioFeed.h"
 
 namespace PokemonAutomation{
 
 class ImageViewRGB32;
 

 int test_pokemonLZA_FlatWhiteDialogDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_BlueDialogDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_TransparentBattleDialogDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_ButtonDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

 int test_pokemonLZA_MainMenuDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_AlertEyeDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_SelectionArrowDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

 int test_pokemonLZA_BoxCellInfoDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

 int test_pokemonLZA_MapIconDetector(const std::string& filepath);

 int test_pokemonLZA_OverworldPartySelectionDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

 int test_pokemonLZA_DirectionArrowDetector(const ImageViewRGB32& image, int target_angle);

int test_pokemonLZA_MapDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLZA_HyperspaceCalorieDetector(const ImageViewRGB32& image, int expected_calorie);

int test_pokemonLZA_FlavorPowerScreenDetector(const ImageViewRGB32& image, bool target);

 }

 #endif
 