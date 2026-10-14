/*  Pixel Sum + Sum of Squares Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Kernels_ImagePixelSumSqr.h"

#ifndef PokemonAutomation_Kernels_ImagePixelSumSqr_TestInfra_H
#define PokemonAutomation_Kernels_ImagePixelSumSqr_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{



using PixelSumSqrConvert = void (*)(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



//
//  Test widths from 0 to 1000, including widths that are and are not
//  divisible by the SIMD vector widths (4, 8, 16 pixels) so that both the
//  SIMD main loop and the partial-word tail are exercised.
//
constexpr size_t kTestWidths[] = {
    0,
    1, 2, 3,
    4, 5, 6, 7,
    8, 9, 10, 11, 12, 13,
    15, 16, 17, 18,
    23, 24, 31, 32, 33,
    40, 47, 48, 49, 56,
    63, 64, 65, 72, 80,
    95, 96, 97, 99, 100,
    112, 127, 128, 129, 131,
    144, 160, 191, 192, 193,
    200, 224, 255, 256, 257,
    288, 320, 352,
    383, 384, 385, 400,
    448, 480,
    511, 512, 513, 544,
    576, 607, 608, 640,
    672, 703, 704,
    736, 767, 768, 769,
    800, 832, 864,
    895, 896, 897, 928,
    959, 960, 991, 992,
    999, 1000,
};



//
//  Warning, this file is compiled in multiple ISA contexts.
//  We cannot define the functions here since they will be compiled differently.
//  If a higher (AVX512) version wins out in the linker, all versions will use
//  it and crash on older CPUs.
//
class Test_PixelSumSqr : public UnitTest{
public:
    Test_PixelSumSqr(
        const std::string& arch_str,
        size_t width, size_t height,
        PixelSumSqrConvert convert
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

private:
    size_t m_width;
    size_t m_height;
    PixelSumSqrConvert m_convert;
};



}
}
#endif
