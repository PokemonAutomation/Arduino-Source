/*  ABS FFT Arch (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_Default_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_Default_H

#include <cmath>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
struct Context_Default{


using vtype = float;
static const int VECTOR_K = 0;
static const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

static const int BASE_COMPLEX_TRANSFORM_K = 4;
static const size_t MIN_TABLE_WIDTH = 1;


static PA_FORCE_INLINE vtype vset1(float x){
    return x;
}
static PA_FORCE_INLINE vtype vneg(vtype x){
    return -x;
}
static PA_FORCE_INLINE vtype vadd(vtype x, vtype y){
    return x + y;
}
static PA_FORCE_INLINE vtype vsub(vtype x, vtype y){
    return x - y;
}
static PA_FORCE_INLINE vtype vmul(vtype x, vtype y){
    return x * y;
}
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = Xi * Wi;
    vtype t1 = Xr * Wi;
    Xr = (Xr * Wr) - t0;
    Xi = (Xi * Wr) + t1;
}
static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    return std::sqrt(r*r + i*i);
}
static PA_FORCE_INLINE void swap_odd(vtype& L, vtype& H){
}

static PA_FORCE_INLINE void interleave_v0(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = lo;
    out1 = hi;
}
static PA_FORCE_INLINE void interleave_v1(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = lo;
    out1 = hi;
}



};
}
}
}
#endif
