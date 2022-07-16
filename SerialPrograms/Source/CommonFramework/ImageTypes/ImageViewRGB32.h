/*  Image View (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_RGB32ImageView_H
#define PokemonAutomation_CommonFramework_RGB32ImageView_H

#include <string>
#include "ImageViewPlanar32.h"

class QImage;

namespace PokemonAutomation{


class ConstImageRef;
class ImageRef;


class ImageRGB32;


class ImageViewRGB32 : public ImageViewPlanar32{
public:
    using ImageViewPlanar32::ImageViewPlanar32;

    ImageViewRGB32(const ConstImageRef& image);
    ImageViewRGB32(const ImageRef& image);

public:
    using ImageViewPlanar32::operator bool;
    using ImageViewPlanar32::data;
    using ImageViewPlanar32::bytes_per_row;
    using ImageViewPlanar32::width;
    using ImageViewPlanar32::height;
    using ImageViewPlanar32::pixel;

    ImageViewRGB32 sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const{
        return ImageViewPlanar32::sub_image(min_x, min_y, width, height);
    }

public:
    ImageRGB32 copy() const;
    void save(const std::string& path) const;

public:
    ImageViewRGB32(const QImage& image);
    QImage to_QImage_ref() const;       //  Return a shallow copy-on-write reference that points to this buffer. (fast)
    QImage to_QImage_owning() const;    //  Return a copy that owns its own buffer. (slow)
    QImage scaled_to_QImage(size_t width, size_t height) const;

private:
    ImageViewRGB32(const ImageViewPlanar32& x)
        : ImageViewPlanar32(x)
    {}
};





}
#endif
