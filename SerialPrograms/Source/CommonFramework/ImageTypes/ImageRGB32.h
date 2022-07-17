/*  Image (RGB 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageRGB32_H
#define PokemonAutomation_CommonFramework_ImageRGB32_H

#include <string>
#include "Common/Cpp/Pimpl.h"
#include "ImageViewRGB32.h"

namespace PokemonAutomation{


class ImageRGB32 : public ImageViewRGB32{
public:
public:
    ~ImageRGB32();
    ImageRGB32(ImageRGB32&& x) noexcept;
    ImageRGB32& operator=(ImageRGB32&& x) noexcept;
private:
    //  Disable these to prevent implicit copying.
    ImageRGB32(const ImageRGB32& x) = delete;
    void operator=(const ImageRGB32& x) = delete;


public:
    ImageRGB32() = default;
    ImageRGB32(size_t width, size_t height);
    ImageRGB32(const std::string& filename);


public:
    using ImageViewRGB32::operator bool;
    using ImageViewRGB32::data;
    using ImageViewRGB32::bytes_per_row;
    using ImageViewRGB32::width;
    using ImageViewRGB32::height;
    using ImageViewRGB32::pixel;
    using ImageViewRGB32::sub_image;


public:
    using ImageViewRGB32::save;


public:
    //  QImage

    ImageRGB32(QImage image);
    using ImageViewRGB32::to_QImage_ref;
    using ImageViewRGB32::to_QImage_owning;
    using ImageViewRGB32::scaled_to_QImage;


private:
    struct Data;
    Pimpl<Data> m_data;
};




}
#endif
