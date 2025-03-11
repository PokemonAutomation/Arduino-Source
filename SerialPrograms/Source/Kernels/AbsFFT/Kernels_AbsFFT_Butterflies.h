/*  ABS FFT Butterflies
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Butterflies_H
#define PokemonAutomation_Kernels_AbsFFT_Butterflies_H

#include "Common/Compiler.h"
#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
template <typename Context>
struct Butterflies{

using vtype = typename Context::vtype;


static PA_FORCE_INLINE void butterfly2(
    vtype& R0, vtype& I0,
    vtype& R1, vtype& I1
){
    vtype r1 = R1;
    vtype i1 = I1;
    R1 = Context::vsub(R0, r1);
    I1 = Context::vsub(I0, i1);
    R0 = Context::vadd(R0, r1);
    I0 = Context::vadd(I0, i1);
}
static PA_FORCE_INLINE void butterfly2(
    vtype& R0, vtype& I0,
    vtype& R1, vtype& I1, vtype W1r, vtype W1i
){
    butterfly2(R0, I0, R1, I1);
    Context::cmul_pp(R1, I1, W1r, W1i);
}

static PA_FORCE_INLINE void butterfly4(
    vtype& R0, vtype& I0,
    vtype& R1, vtype& I1,
    vtype& R2, vtype& I2,
    vtype& R3, vtype& I3
){
    vtype r0, r1, r2, r3;
    vtype i0, i1, i2, i3;

    r0 = Context::vadd(R0, R2);
    r2 = Context::vsub(R0, R2);
    r1 = Context::vadd(R1, R3);
    r3 = Context::vsub(R1, R3);
    i0 = Context::vadd(I0, I2);
    i2 = Context::vsub(I0, I2);
    i1 = Context::vadd(I1, I3);
    i3 = Context::vsub(I1, I3);

    R0 = Context::vadd(r0, r1);
    R1 = Context::vsub(r0, r1);
    I0 = Context::vadd(i0, i1);
    I1 = Context::vsub(i0, i1);
    R2 = Context::vsub(r2, i3);
    R3 = Context::vadd(r2, i3);
    I2 = Context::vadd(i2, r3);
    I3 = Context::vsub(i2, r3);
}
static PA_FORCE_INLINE void butterfly4(
    vtype& R0, vtype& I0,
    vtype& R1, vtype& I1, vtype W1r, vtype W1i,
    vtype& R2, vtype& I2, vtype W2r, vtype W2i,
    vtype& R3, vtype& I3, vtype W3r, vtype W3i
){
    butterfly4(R0, I0, R1, I1, R2, I2, R3, I3);
    Context::cmul_pp(R1, I1, W1r, W1i);
    Context::cmul_pp(R2, I2, W2r, W2i);
    Context::cmul_pp(R3, I3, W3r, W3i);
}


static PA_FORCE_INLINE void reduce2(const TwiddleTable<Context>& table, int k, vtype* T){
    size_t stride = (size_t)1 << (k - 1 - Context::VECTOR_K);
    vtype* T0 = T;
    vtype* T1 = T0 + 2*stride;
    const vcomplex<Context>* w1 = table[k].w1.data();
    size_t lc = stride;
    do{
        vtype r0, r1;
        vtype i0, i1;

        r0 = T0[0];
        i0 = T0[1];
        r1 = T1[0];
        i1 = T1[1];
        Butterflies<Context>::butterfly2(
            r0, i0,
            r1, i1, w1[0].r, w1[0].i
        );
        T0[0] = r0;
        T0[1] = i0;
        T1[0] = r1;
        T1[1] = i1;

        T0 += 2;
        T1 += 2;
        w1++;
    }while (--lc);
}
static PA_FORCE_INLINE void reduce4(const TwiddleTable<Context>& table, int k, vtype* T){
    size_t stride = (size_t)1 << (k - 2 - Context::VECTOR_K);
    vtype* T0 = T;
    vtype* T1 = T0 + 2*stride;
    vtype* T2 = T1 + 2*stride;
    vtype* T3 = T2 + 2*stride;
    const vcomplex<Context>* w1 = table[k - 1].w1.data();
    const vcomplex<Context>* w2 = table[k].w1.data();
    const vcomplex<Context>* w3 = table[k].w3.data();
    size_t lc = stride;
    do{
        vtype r0, r1, r2, r3;
        vtype i0, i1, i2, i3;

        r0 = T0[0];
        i0 = T0[1];
        r1 = T1[0];
        i1 = T1[1];
        r2 = T2[0];
        i2 = T2[1];
        r3 = T3[0];
        i3 = T3[1];
        Butterflies<Context>::butterfly4(
            r0, i0,
            r1, i1, w1[0].r, w1[0].i,
            r2, i2, w2[0].r, w2[0].i,
            r3, i3, w3[0].r, w3[0].i
        );
        T0[0] = r0;
        T0[1] = i0;
        T1[0] = r1;
        T1[1] = i1;
        T2[0] = r2;
        T2[1] = i2;
        T3[0] = r3;
        T3[1] = i3;

        T0 += 2;
        T1 += 2;
        T2 += 2;
        T3 += 2;
        w1++;
        w2++;
        w3++;
    }while (--lc);
}






};
}
}
}
#endif
