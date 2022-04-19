/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageFilter_H
#define PokemonAutomation_CommonFramework_ImageFilter_H

#include "Common/Cpp/Color.h"

class QImage;

namespace PokemonAutomation{

class ConstImageRef;

// If `invert` is false, replace the color outside of the range (`mins` and `maxs`) with the color `replace_with`.
// If `invert` is true, replace the color range (between `mins` and `maxs`) with `replace_with`.
void filter_rgb32_range(QImage& image, uint32_t mins, uint32_t maxs, Color replace_with, bool invert);

void filter1_rgb32_range(
    const ConstImageRef& image,
    QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0
);
void filter2_rgb32_range(
    const ConstImageRef& image,
    QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0,
    QImage& image1, uint32_t mins1, uint32_t maxs1, Color replace_with1, bool invert1
);
void filter4_rgb32_range(
    const ConstImageRef& image,
    QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_with0, bool invert0,
    QImage& image1, uint32_t mins1, uint32_t maxs1, Color replace_with1, bool invert1,
    QImage& image2, uint32_t mins2, uint32_t maxs2, Color replace_with2, bool invert2,
    QImage& image3, uint32_t mins3, uint32_t maxs3, Color replace_with3, bool invert3
);



}
#endif
