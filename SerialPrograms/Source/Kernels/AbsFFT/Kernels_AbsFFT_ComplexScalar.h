/*  ABS FFT Complex Scalar
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_ComplexScalar_H
#define PokemonAutomation_Kernels_AbsFFT_ComplexScalar_H

#include "Kernels_AbsFFT_Arch_Default.h"
#include "Kernels_AbsFFT_TwiddleTable.h"
#include "Kernels_AbsFFT_Butterflies.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



inline void fft_complex_t1_scalar(float T[4]){
    float r0, r1;
    float i0, i1;
    r0 = T[0];
    i0 = T[1];
    r1 = T[2];
    i1 = T[3];
    Butterflies<Context_Default>::butterfly2(r0, i0, r1, i1);
    T[0] = r0;
    T[1] = i0;
    T[2] = r1;
    T[3] = i1;
}
inline void fft_complex_t2_scalar(float T[8]){
    float r0, r1, r2, r3;
    float i0, i1, i2, i3;
    r0 = T[0];
    i0 = T[1];
    r1 = T[2];
    i1 = T[3];
    r2 = T[4];
    i2 = T[5];
    r3 = T[6];
    i3 = T[7];
    Butterflies<Context_Default>::butterfly4(
        r0, i0,
        r1, i1,
        r2, i2,
        r3, i3
    );
    T[0] = r0;
    T[1] = i0;
    T[2] = r1;
    T[3] = i1;
    T[4] = r2;
    T[5] = i2;
    T[6] = r3;
    T[7] = i3;
}
inline void fft_complex_t3_scalar(float T[16]){
    {
        float r0, r1, r2, r3;
        float i0, i1, i2, i3;
        r0 = T[ 0];
        i0 = T[ 1];
        r1 = T[ 4];
        i1 = T[ 5];
        r2 = T[ 8];
        i2 = T[ 9];
        r3 = T[12];
        i3 = T[13];
        Butterflies<Context_Default>::butterfly4(
            r0, i0,
            r1, i1,
            r2, i2,
            r3, i3
        );
        T[ 0] = r0;
        T[ 1] = i0;
        T[ 4] = r1;
        T[ 5] = i1;
        T[ 8] = r2;
        T[ 9] = i2;
        T[12] = r3;
        T[13] = i3;
    }
    {
        float r0, r1, r2, r3;
        float i0, i1, i2, i3;
        r0 = T[ 2];
        i0 = T[ 3];
        r1 = T[ 6];
        i1 = T[ 7];
        r2 = T[10];
        i2 = T[11];
        r3 = T[14];
        i3 = T[15];
        Butterflies<Context_Default>::butterfly4(
            r0, i0,
            r1, i1, 0, 1,
            r2, i2, TW8_1, TW8_1,
            r3, i3, -TW8_1, TW8_1
        );
        T[ 2] = r0;
        T[ 3] = i0;
        T[ 6] = r1;
        T[ 7] = i1;
        T[10] = r2;
        T[11] = i2;
        T[14] = r3;
        T[15] = i3;
    }
    fft_complex_t1_scalar(T +  0);
    fft_complex_t1_scalar(T +  4);
    fft_complex_t1_scalar(T +  8);
    fft_complex_t1_scalar(T + 12);
}




template <typename Context>
void reduce4_scalar(const TwiddleTable<Context>& table, int k, float* T){
    size_t stride = (size_t)1 << (k - 2);
    float* T0 = T;
    float* T1 = T0 + 2*stride;
    float* T2 = T1 + 2*stride;
    float* T3 = T2 + 2*stride;
    TableRowReader<Context> w1(table[k - 1].w1.data());
    TableRowReader<Context> w2(table[k].w1.data());
    TableRowReader<Context> w3(table[k].w3.data());
    size_t c = 0;
    do{
        float r0, r1, r2, r3;
        float i0, i1, i2, i3;
        r0 = T0[0];
        i0 = T0[1];
        r1 = T1[0];
        i1 = T1[1];
        r2 = T2[0];
        i2 = T2[1];
        r3 = T3[0];
        i3 = T3[1];
        float w1r, w1i, w2r, w2i, w3r, w3i;
        w1.get(w1r, w1i, c);
        w2.get(w2r, w2i, c);
        w3.get(w3r, w3i, c);
        Butterflies<Context_Default>::butterfly4(
            r0, i0,
            r1, i1, w1r, w1i,
            r2, i2, w2r, w2i,
            r3, i3, w3r, w3i
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
        c++;
    }while (c < stride);
}


template <typename Context>
void fft_complex_tk_scalar(const TwiddleTable<Context>& table, int k, float* T){
    if (k == 1){
        fft_complex_t1_scalar(T);
        return;
    }
    if (k == 2){
        fft_complex_t2_scalar(T);
        return;
    }
    if (k == 3){
        fft_complex_t3_scalar(T);
        return;
    }

    reduce4_scalar<Context>(table, k, T);

    k -= 2;
    size_t stride = (size_t)2 << k;
    fft_complex_tk_scalar<Context>(table, k, T + 0*stride);
    fft_complex_tk_scalar<Context>(table, k, T + 1*stride);
    fft_complex_tk_scalar<Context>(table, k, T + 2*stride);
    fft_complex_tk_scalar<Context>(table, k, T + 3*stride);
}



}
}
}
#endif
