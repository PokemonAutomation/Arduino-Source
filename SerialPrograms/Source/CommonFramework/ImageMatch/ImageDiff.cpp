/*  Image Diff
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <smmintrin.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/SIMDDebuggers.h"
#include "ImageDiff.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


QRgb pixel_average(const QImage& image){
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    const uint32_t* ref = (const uint32_t*)image.bits();

    int width = image.width();
    int height = image.height();

//    size_t area = 0;
    __m128i sumBG = _mm_setzero_si128();
    __m128i sumRA = _mm_setzero_si128();

    const uint32_t* row = ref;
    for (int r = 0; r < height; r++){
        __m128i sum = _mm_setzero_si128();
        int c = 0;
        while (c < width){
            uint32_t pixel = row[c];
//            area += pixel >> 31;
            __m128i r0 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(pixel));
            sum = _mm_add_epi32(sum, r0);
            c++;
        }

        sumBG = _mm_add_epi64(sumBG, _mm_cvtepu32_epi64(sum));
        sumRA = _mm_add_epi64(sumRA, _mm_cvtepu32_epi64(_mm_unpackhi_epi64(sum, sum)));

        row += words;
    }

    uint64_t B = _mm_cvtsi128_si64(sumBG);
    uint64_t G = _mm_extract_epi64(sumBG, 1);
    uint64_t R = _mm_cvtsi128_si64(sumRA);
    uint64_t A = _mm_extract_epi64(sumRA, 1);

//    cout << "area = " << area << endl;
//    cout << "A = " << A << endl;
//    cout << "R = " << R << endl;
//    cout << "G = " << G << endl;
//    cout << "B = " << B << endl;

    size_t area = A / 255;
    B /= area;
    G /= area;
    R /= area;
    A /= area;

    return qRgba(R, G, B, A);
}
QRgb pixel_average(const QImage& image, const QImage& alpha_mask){
    if (image.size() != alpha_mask.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    if (!image.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }

    size_t image_words = image.bytesPerLine() / sizeof(uint32_t);
    size_t alpha_words = image.bytesPerLine() / sizeof(uint32_t);
    const uint32_t* image_ptr = (const uint32_t*)image.bits();
    const uint32_t* alpha_ptr = (const uint32_t*)alpha_mask.bits();

    int width = image.width();
    int height = image.height();

    size_t area = 0;
    __m128i sumBG = _mm_setzero_si128();
    __m128i sumRA = _mm_setzero_si128();

    const uint32_t* image_row = image_ptr;
    const uint32_t* alpha_row = alpha_ptr;
    for (int r = 0; r < height; r++){
        __m128i sum = _mm_setzero_si128();
        int c = 0;
        while (c < width){
            uint32_t alpha = alpha_row[c];
            uint32_t pixel = image_row[c];
            area += alpha >> 31;
            pixel &= (int32_t)alpha >> 31;
            __m128i r0 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(pixel));
            sum = _mm_add_epi32(sum, r0);
            c++;
        }

        sumBG = _mm_add_epi64(sumBG, _mm_cvtepu32_epi64(sum));
        sumRA = _mm_add_epi64(sumRA, _mm_cvtepu32_epi64(_mm_unpackhi_epi64(sum, sum)));

        image_row += image_words;
        alpha_row += alpha_words;
    }

    uint64_t B = _mm_cvtsi128_si64(sumBG);
    uint64_t G = _mm_extract_epi64(sumBG, 1);
    uint64_t R = _mm_cvtsi128_si64(sumRA);
//    uint64_t A = _mm_extract_epi64(sumRA, 1);

    B /= area;
    G /= area;
    R /= area;

    return qRgb(R, G, B);
}

void scale_brightness(QImage& image, const FloatPixel& multiplier){
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    uint32_t* ref = (uint32_t*)image.bits();

    int width = image.width();
    int height = image.height();

    __m128 scale = _mm_setr_ps((float)multiplier.b, (float)multiplier.g, (float)multiplier.r, 1);

    uint32_t* row = ref;
    for (int r = 0; r < height; r++){
        int c = 0;
        while (c < width){
            uint32_t pixel = row[c];

//            print8(_mm_cvtsi32_si128(pixel)); cout << " ====> ";

            __m128i pi = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(pixel));
            __m128 pf = _mm_cvtepi32_ps(pi);
            pf = _mm_mul_ps(pf, scale);
            pf = _mm_min_ps(pf, _mm_set1_ps(255.));
            pf = _mm_max_ps(pf, _mm_set1_ps(0.));
            pi = _mm_cvtps_epi32(pf);
            pi = _mm_shuffle_epi8(pi, _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

//            print8(_mm_cvtsi32_si128(_mm_cvtsi128_si32(pi))); cout << endl;

            row[c] = _mm_cvtsi128_si32(pi);
            c++;
        }
        row += words;
    }
}




struct RootMeanSquaredDeviation2{
    //  A faster version that's slightly less accurate.

    static const int UNROLL = 4;

    __m128i pixels32 = _mm_setzero_si128();
    __m128i variance64 = _mm_setzero_si128();

    PA_FORCE_INLINE double finish(){
        __m128i x = pixels32;
        x = _mm_add_epi64(_mm_and_si128(x, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(x, 32));
        uint64_t pixels = _mm_cvtsi128_si64(x) + _mm_extract_epi64(x, 1);
        uint64_t variance = _mm_cvtsi128_si64(variance64) + _mm_extract_epi64(variance64, 1);
//        cout << "pixels   = " << pixels << endl;
//        cout << "variance = " << variance << endl;
        return std::sqrt((double)variance / (double)pixels);
    }
    PA_FORCE_INLINE void process(
        __m128i r, __m128i i
    ){
        //  Count active pixels.
        pixels32 = _mm_add_epi32(pixels32, _mm_srli_epi32(r, 31));

        __m128i rA = _mm_shuffle_epi8(r, _mm_setr_epi8(3, 3, 3, -1, 7, 7, 7, -1, 11, 11, 11, -1, 15, 15, 15, -1));

        r = _mm_srli_epi16(r, 2);
        i = _mm_srli_epi16(i, 2);
        r = _mm_and_si128(r, _mm_set1_epi8(0x3f));
        i = _mm_and_si128(i, _mm_set1_epi8(0x3f));

        //  Absolute value subtract.
//        i = _mm_max_epi8(_mm_sub_epi8(r, i), _mm_sub_epi8(i, r));
        i = _mm_sub_epi8(r, i);
        i = _mm_sign_epi8(i, i);

        //  Zero if both alphas are zero.
        i = _mm_and_si128(i, rA);

        //  Square
        i = _mm_maddubs_epi16(i, i);

        //  Reduce
        i = _mm_madd_epi16(i, _mm_set1_epi16(16));
        i = _mm_add_epi64(_mm_and_si128(i, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(i, 32));
        variance64 = _mm_add_epi64(variance64, i);
    }
    PA_FORCE_INLINE void process1(
        const uint32_t ref[1], const uint32_t img[1]
    ){
        process(_mm_cvtsi32_si128(*ref), _mm_cvtsi32_si128(*img));
    }
    PA_FORCE_INLINE void processU(
        const uint32_t ref[4], const uint32_t img[4]
    ){
        process(
            _mm_loadu_si128((const __m128i*)ref),
            _mm_loadu_si128((const __m128i*)img)
        );
    }

};
struct RootMeanSquaredDeviationWithBackground{

    RootMeanSquaredDeviationWithBackground(QRgb p_background){
        background = _mm_set1_epi32(p_background);
    }

    static const int UNROLL = 4;

    __m128i background;
    __m128i pixels32 = _mm_setzero_si128();
    __m128i variance64 = _mm_setzero_si128();

    PA_FORCE_INLINE double finish(){
        __m128i x = pixels32;
        x = _mm_add_epi64(_mm_and_si128(x, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(x, 32));
        uint64_t pixels = _mm_cvtsi128_si64(x) + _mm_extract_epi64(x, 1);
        uint64_t variance = _mm_cvtsi128_si64(variance64) + _mm_extract_epi64(variance64, 1);
//        cout << "pixels   = " << pixels << endl;
//        cout << "variance = " << variance << endl;
        return std::sqrt((double)variance / (double)pixels);
    }
    PA_FORCE_INLINE void process(
        __m128i r, __m128i i
    ){
//        print8(r); cout << " : "; print8(i); cout << endl;

        //  Count active pixels.
        pixels32 = _mm_add_epi32(pixels32, _mm_srli_epi32(r, 31));

        //  Replace transparent pixels with background.
        __m128i rA = _mm_srai_epi32(r, 31);
        r = _mm_blendv_epi8(background, r, rA);

        r = _mm_srli_epi16(r, 2);
        i = _mm_srli_epi16(i, 2);
        r = _mm_and_si128(r, _mm_set1_epi8(0x3f));
        i = _mm_and_si128(i, _mm_set1_epi8(0x3f));

        //  Absolute value subtract.
        i = _mm_sub_epi8(r, i);
        i = _mm_sign_epi8(i, i);

        i = _mm_and_si128(i, _mm_set1_epi32(0x00ffffff));
//        print8(i); cout << endl;

        //  Square
        i = _mm_maddubs_epi16(i, i);

        //  Reduce
        i = _mm_madd_epi16(i, _mm_set1_epi16(16));
        i = _mm_add_epi64(_mm_and_si128(i, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(i, 32));
        variance64 = _mm_add_epi64(variance64, i);
    }
    PA_FORCE_INLINE void process1(
        const uint32_t ref[1], const uint32_t img[1]
    ){
        __m128i r = _mm_insert_epi32(background, *ref, 0);
        __m128i i = _mm_insert_epi32(background, *img, 0);
        process(r, i);
    }
    PA_FORCE_INLINE void processU(
        const uint32_t ref[4], const uint32_t img[4]
    ){
        process(
            _mm_loadu_si128((const __m128i*)ref),
            _mm_loadu_si128((const __m128i*)img)
        );
    }

};
struct RootMeanSquaredDeviationMasked2{
    //  A faster version that's slightly less accurate.

    static const int UNROLL = 4;

    __m128i pixels32 = _mm_setzero_si128();
    __m128i variance64 = _mm_setzero_si128();

    PA_FORCE_INLINE double finish(){
        __m128i x = pixels32;
        x = _mm_add_epi64(_mm_and_si128(x, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(x, 32));
        uint64_t pixels = _mm_cvtsi128_si64(x) + _mm_extract_epi64(x, 1);
        uint64_t variance = _mm_cvtsi128_si64(variance64) + _mm_extract_epi64(variance64, 1);
//        cout << "pixels   = " << pixels << endl;
//        cout << "variance = " << variance << endl;
        return std::sqrt((double)variance / (double)pixels);
    }
    PA_FORCE_INLINE void process(
        __m128i r, __m128i i
    ){
//        print8(r); cout << " : "; print8(i); cout << endl;

        //  Count active pixels.
        pixels32 = _mm_add_epi32(pixels32, _mm_srli_epi32(r, 31));

        r = _mm_srli_epi16(r, 2);
        i = _mm_srli_epi16(i, 2);
        r = _mm_and_si128(r, _mm_set1_epi8(0x3f));
        i = _mm_and_si128(i, _mm_set1_epi8(0x3f));

        __m128i rA = _mm_shuffle_epi8(r, _mm_setr_epi8(3, 3, 3, -1, 7, 7, 7, -1, 11, 11, 11, -1, 15, 15, 15, -1));
        __m128i iA = _mm_shuffle_epi8(i, _mm_setr_epi8(3, 3, 3, -1, 7, 7, 7, -1, 11, 11, 11, -1, 15, 15, 15, -1));

        //  Absolute value subtract.
//        i = _mm_max_epi8(_mm_sub_epi8(r, i), _mm_sub_epi8(i, r));
        i = _mm_sub_epi8(r, i);
        i = _mm_sign_epi8(i, i);

        //  Zero if both alphas are zero.
        i = _mm_and_si128(i, _mm_or_si128(rA, iA));

        //  Max out if alphas mismatch.
        i = _mm_or_si128(i, _mm_andnot_si128(rA, iA));

        //  Square
        i = _mm_maddubs_epi16(i, i);
//        print16(i); cout << endl;

        //  Reduce
        i = _mm_madd_epi16(i, _mm_set1_epi16(16));
//        print32(i); cout << endl;

        i = _mm_add_epi64(_mm_and_si128(i, _mm_set1_epi64x(0xffffffff)), _mm_srli_epi64(i, 32));
        variance64 = _mm_add_epi64(variance64, i);
    }
    PA_FORCE_INLINE void process1(
        const uint32_t ref[1], const uint32_t img[1]
    ){
        process(_mm_cvtsi32_si128(*ref), _mm_cvtsi32_si128(*img));
    }
    PA_FORCE_INLINE void processU(
        const uint32_t ref[4], const uint32_t img[4]
    ){
        process(
            _mm_loadu_si128((const __m128i*)ref),
            _mm_loadu_si128((const __m128i*)img)
        );
    }

};


template <typename Accumulator, class... Args>
double pixeldiff(const QImage& reference, const QImage& image, Args&&... args){
    if (reference.size() != image.size()){
        PA_THROW_StringException("Mismatching Dimensions");
    }
    if (!reference.size().isValid()){
        PA_THROW_StringException("Invalid Dimensions");
    }

    const QImage& formatted = image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32
        ? image
        : image.convertToFormat(QImage::Format_RGB32);

    size_t wordsR = reference.bytesPerLine() / sizeof(uint32_t);
    size_t wordsI = formatted.bytesPerLine() / sizeof(uint32_t);
    const uint32_t* ref = (const uint32_t*)reference.bits();
    const uint32_t* img = (const uint32_t*)formatted.bits();

    int width = reference.width();
    int height = reference.height();

    Accumulator accumulator(std::forward<Args>(args)...);

    const uint32_t* ref_row = ref;
    const uint32_t* img_row = img;
    for (int r = 0; r < height; r++){
        int c = 0;
        while (c + Accumulator::UNROLL - 1 < width){
            accumulator.processU(ref_row + c, img_row + c);
            c += Accumulator::UNROLL;
        }
        while (c < width){
            accumulator.process1(ref_row + c, img_row + c);
            c++;
        }
        ref_row += wordsR;
        img_row += wordsI;
//        cout << "---------------" << endl;
    }

    return accumulator.finish();
}

double pixel_RMSD(const QImage& reference, const QImage& image){
    return pixeldiff<RootMeanSquaredDeviation2>(reference, image);
}
double pixel_RMSD(const QImage& reference, const QImage& image, QRgb background){
    return pixeldiff<RootMeanSquaredDeviationWithBackground>(reference, image, background);
}
double pixel_RMSD_masked(const QImage& reference, const QImage& image){
    return pixeldiff<RootMeanSquaredDeviationMasked2>(reference, image);
}



}
}
