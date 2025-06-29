/*  Filter to Alpha
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "FilterToAlpha.h"

#ifdef PA_ARCH_x86
#include <smmintrin.h>
#endif

namespace PokemonAutomation{
namespace ImageMatch{



void set_alpha_black(ImageRGB32& image, uint32_t max_rgb_sum){
    size_t words = image.bytes_per_row() / sizeof(uint32_t);
    uint32_t* data = image.data();

    size_t width = image.width();
    size_t height = image.height();

#ifdef PA_ARCH_x86
    __m128i threshold = _mm_set1_epi32(max_rgb_sum);
#endif

    uint32_t* row = data;
    for (size_t r = 0; r < height; r++){
        size_t c = 0;
#ifdef PA_ARCH_x86
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
            // Set alpha channel to 0.
            pixel &= 0x00ffffff;

            uint32_t sum = pixel & 0xff;
            sum += (pixel >>  8) & 0xff;
            sum += (pixel >> 16) & 0xff;
            if (sum <= max_rgb_sum){
                // If dark, set alpha channel to 255.
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
