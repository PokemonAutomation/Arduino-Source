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
 
 }
 
 #endif
 