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
 
 
 int test_pokemonZLA_NormalDialogBoxDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_FlatWhiteDialogDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_BlueDialogDetector(const ImageViewRGB32& image, bool target);

 int test_pokemonLZA_ButtonDetector(const ImageViewRGB32& image, const std::vector<std::string>& words);

 int test_pokemonLZA_MainMenuDetector(const ImageViewRGB32& image, bool target);

 }

 #endif
 