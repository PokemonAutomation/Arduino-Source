/*  Image (RGB32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageRGB32_H
#define PokemonAutomation_CommonFramework_ImageRGB32_H

#include <memory>
#include <string>
#include "Common/Cpp/Containers/AlignedVector.h"
#include "ImageViewRGB32.h"

namespace PokemonAutomation{



class CustomImageRGB32Owner{
public:
    virtual ~CustomImageRGB32Owner() = default;
    virtual ImageViewRGB32 get_view() const = 0;
};




class ImageRGB32 : public ImageViewRGB32{
public:
    ~ImageRGB32();
    ImageRGB32(ImageRGB32&& x) noexcept;
    ImageRGB32& operator=(ImageRGB32&& x) noexcept;
private:
    //  Disable these to prevent implicit copying.
    ImageRGB32(const ImageRGB32& x) = delete;
    void operator=(const ImageRGB32& x) = delete;


public:
    ImageRGB32();

    //  Create an ARGB32 image of shape width x height with uninitialized pixels.
    ImageRGB32(size_t width, size_t height);

    explicit ImageRGB32(const std::string& filename);

    ImageRGB32(std::unique_ptr<CustomImageRGB32Owner> image);


    //  Fill the entire image with the specified pixel.
    using ImageViewPlanar32::fill;


public:
    //  Returns true if this image is valid. (non-null and non-zero dimensions)
    using ImageViewRGB32::operator bool;

    const uint32_t* data() const{ return m_ptr; }
          uint32_t* data(){ return m_ptr; }

    using ImageViewRGB32::bytes_per_row;
    using ImageViewRGB32::width;
    using ImageViewRGB32::height;

    //  Direct Pixel Access
    PA_FORCE_INLINE uint32_t pixel(size_t x, size_t y) const{
        return ImageViewPlanar32::pixel(x, y);
    }
    PA_FORCE_INLINE uint32_t& pixel(size_t x, size_t y){
        return ImageViewPlanar32::pixel(x, y);
    }

public:
    using ImageViewRGB32::sub_image;
    using ImageViewRGB32::save;


private:
    AlignedVector<uint32_t> m_data;
    std::unique_ptr<CustomImageRGB32Owner> m_custom_owner;
};











}
#endif
