/*  Filter to Alpha
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_FilterToAlpha_H
#define PokemonAutomation_CommonTools_FilterToAlpha_H

#include <stdint.h>

namespace PokemonAutomation{
    class ImageRGB32;
namespace ImageMatch{



//  For each pixel, set the alpha channel to 255 if the RGB sum is no greater than "max_rgb_sum".
//  The rest pixels are set to 0 alpha.
//  This function is fast.
void set_alpha_black(ImageRGB32& image, uint32_t max_rgb_sum = 100);


}
}
#endif
