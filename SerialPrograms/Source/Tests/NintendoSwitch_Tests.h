/*  Nintendo Switch Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_NintendoSwitch_Tests_H
#define PokemonAutomation_Tests_NintendoSwitch_Tests_H

namespace PokemonAutomation{

class ImageViewRGB32;

int test_NintendoSwitch_CheckOnlineDetector(const ImageViewRGB32& image, bool target);
int test_NintendoSwitch_FailedToConnectDetector(const ImageViewRGB32& image, bool target);
int test_NintendoSwitch_UpdatePopupDetector(const ImageViewRGB32& image, bool target);

}

#endif
