/*  ABS FFT Arch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_H

#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


const float TW8_1 = 0.70710678118654752440f;

#if 0
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
#endif


template <typename Context>
struct vcomplex{
    using vtype = typename Context::vtype;

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
