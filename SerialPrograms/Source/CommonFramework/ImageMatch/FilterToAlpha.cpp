/*  Filter to Alpha
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/SIMDDebuggers.h"
#include "FilterToAlpha.h"

namespace PokemonAutomation{
namespace ImageMatch{


QImage black_filter_to_alpha(const QImage& image, uint32_t max_rgb_sum){
    QImage filtered = image.convertToFormat(QImage::Format_ARGB32);

    int width = filtered.width();
    int height = filtered.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            QRgb pixel = filtered.pixel(c, r);
            uint32_t sum = qRed(pixel) + qGreen(pixel) + qBlue(pixel);
            pixel &= 0x00ffffff;
            if (sum <= max_rgb_sum){
                pixel |= 0xff000000;
//                pixel = 0xff000000;
            }else{
//                pixel = 0x00ffffff;
            }
            filtered.setPixel(c, r, pixel);
        }
    }

    return filtered;
}


void set_alpha_black(QImage& image, uint32_t max_rgb_sum){
    image = image.convertToFormat(QImage::Format_ARGB32);

    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    uint32_t* data = (uint32_t*)image.bits();

    int width = image.width();
    int height = image.height();

    __m128i threshold = _mm_set1_epi32(max_rgb_sum);

    uint32_t* row = data;
    for (int r = 0; r < height; r++){
        int c = 0;
#if 1
        while (c + 3 < width){
            __m128i pixel = _mm_loadu_si128((__m128i*)(row + c));
//            print8(pixel); cout << " ====> ";

            pixel = _mm_andnot_si128(_mm_set1_epi32(0xff000000), pixel);

            __m128i sum = _mm_and_si128(pixel, _mm_set1_epi32(0x000000ff));
            sum = _mm_add_epi32(sum, _mm_and_si128(_mm_srli_epi32(pixel, 8), _mm_set1_epi32(0x000000ff)));
            sum = _mm_add_epi32(sum, _mm_and_si128(_mm_srli_epi32(pixel, 16), _mm_set1_epi32(0x000000ff)));
            sum = _mm_cmpgt_epi32(sum, threshold);
            sum = _mm_andnot_si128(sum, _mm_set1_epi32(0xff000000));
            pixel = _mm_or_si128(pixel, sum);

//            print8(pixel); cout << endl;

            _mm_storeu_si128((__m128i*)(row + c), pixel);
            c += 4;
        }
#endif
        while (c < width){
            uint32_t pixel = row[c];
            pixel &= 0x00ffffff;

            uint32_t sum = pixel & 0xff;
            sum += (pixel >>  8) & 0xff;
            sum += (pixel >> 16) & 0xff;
            if (sum <= max_rgb_sum){
                pixel |= 0xff000000;
            }
            row[c] = pixel;
            c++;
        }


        row += words;
    }
}



}
}
