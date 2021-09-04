/*  Filter to Alpha
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_FilterToAlpha_H
#define PokemonAutomation_CommonFramework_FilterToAlpha_H

#include <functional>
#include <QImage>

namespace PokemonAutomation{
namespace ImageMatch{


//  Set the alpha channels to 255 for every pixel that is sufficiently black.
//  This function is slow.
QImage black_filter_to_alpha(const QImage& image, uint32_t max_rgb_sum = 100);


//  For each pixel, set the alpha channel if the RGB sum is no greater than "max_rgb_sum".
//  This function is fast.
void set_alpha_black(QImage& image, uint32_t max_rgb_sum = 100);


}
}
#endif
