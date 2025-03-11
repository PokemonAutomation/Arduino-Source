/*  Image View (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageViewHSV32_H
#define PokemonAutomation_CommonFramework_ImageViewHSV32_H

#include <string>
#include "ImageViewPlanar32.h"

namespace PokemonAutomation{


class ImageHSV32;


class ImageViewHSV32 : public ImageViewPlanar32{
public:
    using ImageViewPlanar32::ImageViewPlanar32;

public:
    using ImageViewPlanar32::operator bool;

    using ImageViewPlanar32::data;
    using ImageViewPlanar32::bytes_per_row;
    using ImageViewPlanar32::width;
    using ImageViewPlanar32::height;

    using ImageViewPlanar32::pixel;

    ImageViewHSV32 sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
        return ImageViewPlanar32::sub_image(min_x, min_y, width, height);
    }

public:
    ImageHSV32 copy() const;

private:
    ImageViewHSV32(const ImageViewPlanar32& x)
        : ImageViewPlanar32(x)
    {}
};





}
#endif
