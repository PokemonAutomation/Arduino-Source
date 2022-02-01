/*  ABS FFT Arch
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_H

#include "Kernels/Kernels_Arch.h"

#if 0
//  Arch Override
//#define PA_Kernels_AbsFFT_Arch_x86_AVX2
//#define PA_Kernels_AbsFFT_Arch_x86_SSE41
#define PA_Kernels_AbsFFT_Arch_Default

#else
#if 0
#elif defined PA_Arch_x64_AVX2
#define PA_Kernels_AbsFFT_Arch_x86_AVX2
#elif defined PA_Arch_x64_SSE42
#define PA_Kernels_AbsFFT_Arch_x86_SSE41
#else
#define PA_Kernels_AbsFFT_Arch_Default
#endif
#endif


#include "Kernels_AbsFFT_Arch_Default.h"
#ifdef PA_Kernels_AbsFFT_Arch_x86_SSE41
#include "Kernels_AbsFFT_Arch_x86_SSE41.h"
#endif
#ifdef PA_Kernels_AbsFFT_Arch_x86_AVX2
#include "Kernels_AbsFFT_Arch_x86_AVX2.h"
#endif


namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{

#if 0
#elif defined PA_Kernels_AbsFFT_Arch_x86_AVX2
using Intrinsics = Intrinsics_x86_AVX2;
#elif defined PA_Kernels_AbsFFT_Arch_x86_SSE41
using Intrinsics = Intrinsics_x86_SSE41;
#elif defined PA_Kernels_AbsFFT_Arch_Default
using Intrinsics = Intrinsics_Default;
#endif

using vtype = Intrinsics::vtype;

const size_t VECTOR_K = Intrinsics::VECTOR_K;
const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

const float TW8_1 = 0.70710678118654752440f;

PA_FORCE_INLINE void cmul_pp(
    float& Or, float& Oi,
    float Xr, float Xi,
    float Wr, float Wi
){
    Or = Xr*Wr - Xi*Wi;
    Oi = Xr*Wi + Xi*Wr;
}

struct scomplex{
    float r;
    float i;

    friend PA_FORCE_INLINE scomplex operator+(const scomplex& x, const scomplex& y){
        return scomplex{x.r + y.r, x.i + y.i};
    }
    friend PA_FORCE_INLINE scomplex operator-(const scomplex& x, const scomplex& y){
        return scomplex{x.r - y.r, x.i - y.i};
    }
    friend PA_FORCE_INLINE scomplex operator*(const scomplex& x, const scomplex& y){
        return scomplex{x.r*y.r - x.i*y.i, x.r*y.i + x.i*y.r};
    }

    scomplex mul_by_i() const{
        return scomplex{-i, r};
    }
};

struct vcomplex{
    using vtype = Intrinsics::vtype;

    vtype r;
    vtype i;

    //  Suppress compiler memset/memcpy optimization.
    PA_FORCE_INLINE vcomplex() = default;
    PA_FORCE_INLINE vcomplex(const vcomplex& x)
        : r(x.r)
        , i(x.i)
    {}
    PA_FORCE_INLINE void operator=(const vcomplex& x){
        r = x.r;
        i = x.i;
    }

    float  real(size_t index) const{ return ((const float*)&r)[index]; }
    float& real(size_t index)      { return ((      float*)&r)[index]; }
    float  imag(size_t index) const{ return ((const float*)&i)[index]; }
    float& imag(size_t index)      { return ((      float*)&i)[index]; }
};


}
}
}
#endif
