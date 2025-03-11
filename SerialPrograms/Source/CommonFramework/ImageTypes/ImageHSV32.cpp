/*  Image (HSV 32)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <utility>
#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "ImageViewRGB32.h"
#include "ImageViewHSV32.h"
#include "ImageHSV32.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// #include <iostream>
// using std::cout;
// using std::endl;

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
    : ImageViewHSV32(width, height)
    , m_data(CONSTRUCT_TOKEN, m_bytes_per_row / sizeof(uint32_t) * height)
{
    m_ptr = m_data->self.data();
}


uint32_t hsv_to_rgb(uint32_t p){
    int r = (uint32_t(0xff) & (p >> 16));
    int g = (uint32_t(0xff) & (p >> 8));
    int b = (uint32_t(0xff) & p);

    int M = std::max(std::max(r, g), b);
    int m = std::min(std::min(r, g), b);

    int delta = M - m;

    // cout << "r=" << r << " g=" << g << " b=" << b << endl;
    // cout << "M=" << M << " m=" << m << " delta=" << delta << endl;

    int S = 0;
    if (M > 0){
        S = std::min(std::max(255 - (m*255 + M/2)/M, 0), 255);
    }

    int V = M;

    double Hf = 0;
    if (delta > 0){
        if (M == r){
            Hf = fmod((g - b)/(double)delta, 6.0);
        }else if (M == g){
            Hf = (b - r)/(double)delta + 2.0;
        }else{
            Hf = (r - g)/(double)delta + 4.0;
        }
    }
    // cout << "H standard = " << Hf * 60.0 << endl;
    // This Hf * 60.0 is the standard H value, which ranges in [0, 360).
    // To hold it in a uint8, need to convert its range to [0, 255]
    int H = std::max(int(Hf * 256.0 / 6.0 + 0.5) % 256, 0);
    // cout << "H=" << H << ", S=" << S << ", M=" << M << endl;

    return (p & 0xff000000) |
           ((uint32_t)(uint8_t)H << 16) |
           ((uint32_t)(uint8_t)S << 8) |
           (uint8_t)V;
}


ImageHSV32::ImageHSV32(const ImageViewRGB32& image)
    : ImageViewHSV32(image.width(), image.height())
    , m_data(CONSTRUCT_TOKEN, m_bytes_per_row / sizeof(uint32_t) * m_height)
{
    m_ptr = m_data->self.data();

    // {
    //     // XXX
    //     // auto p = combine_rgb(173,238,112);
    //     auto p = combine_rgb(200,255,133);
    //     auto p2 = hsv_to_rgb(p);
    //     auto h = (p2 & 0x00ff0000) >> 16;
    //     auto s = (p2 & 0x0000ff00) >> 8;
    //     auto v = (p2 & 0x000000ff);
    //     cout << "h " << h << " s " << s << " v " << v << endl;
    //     exit(0);
    // }

    size_t width = image.width();
    size_t height = image.height();
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            uint32_t p = image.pixel(x, y);

            this->pixel(x, y) = hsv_to_rgb(p);
        }
    }
}







}
