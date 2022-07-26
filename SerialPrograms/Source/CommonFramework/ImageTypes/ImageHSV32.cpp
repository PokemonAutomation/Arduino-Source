/*  Image (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <utility>
#include <cmath>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/AlignedVector.tpp"
#include "ImageViewRGB32.h"
#include "ImageViewHSV32.h"
#include "ImageHSV32.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PokemonAutomation{

struct ImageHSV32::Data{
    AlignedVector<uint32_t> self;

    Data(size_t items) : self(items) {}
};



ImageHSV32::~ImageHSV32() = default;
ImageHSV32::ImageHSV32(ImageHSV32&& x) noexcept{
    *this = std::move(x);
}
ImageHSV32& ImageHSV32::operator=(ImageHSV32&& x) noexcept{
    if (this != &x){
        ImageViewHSV32::operator=(x);
        m_data = std::move(x.m_data);
        x.m_bytes_per_row = 0;
        x.m_ptr = nullptr;
        x.m_width = 0;
        x.m_height = 0;
    }
    return *this;
}

ImageHSV32::ImageHSV32() = default;

ImageHSV32::ImageHSV32(size_t width, size_t height)
    : ImageViewHSV32(nullptr, (width * sizeof(uint32_t) + PA_ALIGNMENT - 1) & ~(size_t)(PA_ALIGNMENT - 1), width, height)
    , m_data(CONSTRUCT_TOKEN, m_bytes_per_row / sizeof(uint32_t) * height)
{
    m_ptr = m_data->self.data();
}



ImageHSV32::ImageHSV32(const ImageViewRGB32& image)
    : ImageViewHSV32(
        nullptr, (image.width() * sizeof(uint32_t) + PA_ALIGNMENT - 1) & ~(size_t)(PA_ALIGNMENT - 1),
        image.width(), image.height()
    )
    , m_data(CONSTRUCT_TOKEN, m_bytes_per_row / sizeof(uint32_t) * m_height)
{
    size_t width = image.width();
    size_t height = image.height();
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            uint32_t p = image.pixel(x, y);

            int r = (uint32_t(0xff) & (p >> 16));
            int g = (uint32_t(0xff) & (p >> 8));
            int b = (uint32_t(0xff) & p);

            int M = std::max(std::max(r, g), b);
            int m = std::min(std::min(r, g), b);

            int S = 0;
            if (M > 0){
                S = std::min(std::max(255 - (m*255 + M/2)/M, 0), 255);
            }

            double cosH = std::sqrt(r*r + g*g + b*b - r*g - r*b - g*b) * (r - 0.5*g - 0.5*b);
            double Hf = std::acos(std::min(std::max(cosH, -1.0), 1.0)) * (180 / M_PI);
            if (b > g){
                Hf = 360 - Hf;
            }
            // Convert Hf range from [0, 360) to [0, 256)
            int H = std::max(int(Hf * 256.0 / 360.0 + 0.5) % 256, 0);

            this->pixel(x, y) =
                (p & 0xff000000) |
                ((uint32_t)(uint8_t)H << 16) |
                ((uint32_t)(uint8_t)S << 8) |
                (uint8_t)M;
        }
    }
}







}
