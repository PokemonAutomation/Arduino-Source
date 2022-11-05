/*  Nintendo Switch Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_NintendoSwitch_Tests_H
#define PokemonAutomation_Tests_NintendoSwitch_Tests_H

namespace PokemonAutomation{

class ImageViewRGB32;

int test_NintendoSwitch_UpdateMenuDetector(const ImageViewRGB32& image, bool target);

}

#endif
