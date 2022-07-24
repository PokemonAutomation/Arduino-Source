/*  Image View (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageViewHSV32_H
#define PokemonAutomation_CommonFramework_ImageViewHSV32_H

#include <string>
#include "ImageViewPlanar32.h"

class QImage;

namespace PokemonAutomation{


class ConstImageRef;
class ImageRef;


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
    bool save(const std::string& path) const;
    ImageHSV32 scale_to(size_t width, size_t height) const;

public:
    ImageViewHSV32(const QImage& image);
    QImage to_QImage_ref() const;       //  Return a shallow copy-on-write reference that points to this buffer. (fast)
    QImage to_QImage_owning() const;    //  Return a copy that owns its own buffer. (slow)

private:
    ImageViewHSV32(const ImageViewPlanar32& x)
        : ImageViewPlanar32(x)
    {}
};





}
#endif
