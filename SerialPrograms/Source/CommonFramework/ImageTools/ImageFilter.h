/*  Image Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageFilter_H
#define PokemonAutomation_CommonFramework_ImageFilter_H

#include <vector>
#include "Common/Cpp/Color.h"

class QImage;

namespace PokemonAutomation{

class ConstImageRef;

//  If `invert` is false, replace the color outside of the range [mins, maxs] with the color `replace_with`.
//  If `invert` is true, replace the color range [mins, maxs] with `replace_with`.
//  Returns the # of pixels inside the range [mins, maxs].
size_t filter_rgb32_range(QImage& image, uint32_t mins, uint32_t maxs, Color replace_with, bool invert);



struct FilterRgb32Range{
    uint32_t mins;
    uint32_t maxs;
    Color replace_with;
    bool invert;
};
std::vector<std::pair<QImage, size_t>> filter_rgb32_range(
    const ConstImageRef& image,
    const std::vector<FilterRgb32Range>& filters
);



//void bucketize1_rgb32_range(
//    const ConstImageRef& image,
//    size_t& count0, QImage& image0, uint32_t mins0, uint32_t maxs0, Color replace_in, Color replace_out
//);




}
#endif
