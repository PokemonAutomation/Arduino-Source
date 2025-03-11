/*  ABS FFT Bit Reverse
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BitReverse_H
#define PokemonAutomation_Kernels_AbsFFT_BitReverse_H

#include <stdint.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


template <typename Context>
struct BitReverse{


using vtype = typename Context::vtype;


static PA_FORCE_INLINE void interleave_f32_scalar(int len_k, float* out, const float* in){
    size_t stride = (size_t)1 << (len_k - 1);
    const float* in0 = in;
    const float* in1 = in + stride;
    size_t lc = stride;
    do{
        float r0 = in0[0];
        float r1 = in1[0];
        out[0] = r0;
        out[1] = r1;
        in0 += 1;
        in1 += 1;
        out += 2;
    }while (--lc);
}
static PA_FORCE_INLINE void interleave_u64_scalar(int len_k, uint64_t* out, const uint64_t* in){
    size_t stride = (size_t)1 << (len_k - 1);
    const uint64_t* in0 = in;
    const uint64_t* in1 = in + stride;
    size_t lc = stride;
    do{
        uint64_t r0 = in0[0];
        uint64_t r1 = in1[0];
        out[0] = r0;
        out[1] = r1;
        in0++;
        in1++;
        out += 2;
    }while (--lc);
}
static PA_FORCE_INLINE void interleave_u64(int len_k, uint64_t* out, const uint64_t* in){
    if ((size_t)len_k < Context::VECTOR_K || Context::VECTOR_K == 0){
        interleave_u64_scalar(len_k, out, in);
        return;
    }
    size_t vstride = (size_t)1 << (len_k - Context::VECTOR_K);
    const vtype* in0 = (const vtype*)in;
    const vtype* in1 = in0 + vstride;
    vtype* out0 = (vtype*)out;
    size_t lc = vstride;
    do{
        vtype r0, r1;
        Context::interleave_v1(r0, r1, in0[0], in1[0]);
        out0[0] = r0;
        out0[1] = r1;
        in0 += 1;
        in1 += 1;
        out0 += 2;
    }while (--lc);
}




//  32-bit Granular
static void interleave_f32(int len_k, float* out, const float* in){
    using vtype = typename Context::vtype;

    if ((size_t)len_k < Context::VECTOR_K + 1){
        interleave_f32_scalar(len_k, (float*)out, (const float*)in);
        return;
    }
    size_t vstride = (size_t)1 << (len_k - 1 - Context::VECTOR_K);
    const vtype* in0 = (const vtype*)in;
    const vtype* in1 = in0 + vstride;
    vtype* out0 = (vtype*)out;
    size_t lc = vstride;
    do{
        vtype r0, r1;
        Context::interleave_v0(r0, r1, in0[0], in1[0]);
        out0[0] = r0;
        out0[1] = r1;
        in0 += 1;
        in1 += 1;
        out0 += 2;
    }while (--lc);
}


//  64-bit Granular

//  in-place
static void bitreverse_u64_ip(int len_k, uint64_t* data, uint64_t* temp){
    if (len_k <= 1){
        return;
    }
    if (len_k == 2){
        uint64_t r1 = data[1];
        uint64_t r2 = data[2];
        data[1] = r2;
        data[2] = r1;
        return;
    }
    if (len_k == 3){
        uint64_t r1 = data[1];
        uint64_t r3 = data[3];
        uint64_t r4 = data[4];
        uint64_t r6 = data[6];
        data[1] = r4;
        data[3] = r6;
        data[4] = r1;
        data[6] = r3;
        return;
    }

    len_k--;
    size_t stride = (size_t)1 << len_k;
    bitreverse_u64_np(len_k, temp + 0*stride, data + 0*stride);
    bitreverse_u64_np(len_k, temp + 1*stride, data + 1*stride);
    interleave_u64(len_k + 1, data, temp);
}

//  out-of-place
static void bitreverse_u64_np(int len_k, uint64_t* out, uint64_t* in){
    if (len_k == 0){
        out[0] = in[0];
        return;
    }
    if (len_k == 1){
        out[0] = in[0];
        out[1] = in[1];
        return;
    }
    if (len_k == 2){
        uint64_t r0 = in[0];
        uint64_t r1 = in[1];
        uint64_t r2 = in[2];
        uint64_t r3 = in[3];
        out[0] = r0;
        out[1] = r2;
        out[2] = r1;
        out[0] = r3;
        return;
    }
    if (len_k == 3){
        uint64_t r0 = in[0];
        uint64_t r1 = in[1];
        uint64_t r2 = in[2];
        uint64_t r3 = in[3];
        uint64_t r4 = in[4];
        uint64_t r5 = in[5];
        uint64_t r6 = in[6];
        uint64_t r7 = in[7];
        out[0] = r0;
        out[1] = r4;
        out[2] = r2;
        out[3] = r6;
        out[4] = r1;
        out[5] = r5;
        out[6] = r3;
        out[7] = r7;
        return;
    }

    len_k--;
    size_t stride = (size_t)1 << len_k;
    bitreverse_u64_ip(len_k, in + 0*stride, out + 0*stride);
    bitreverse_u64_ip(len_k, in + 1*stride, out + 1*stride);
    interleave_u64(len_k + 1, out, in);
}

static inline void bitreverse_f32v1_ip(int len_k, float* data, float* temp){
    bitreverse_u64_ip(len_k - 1, (uint64_t*)data, (uint64_t*)temp);
}


};
}
}
}
#endif
