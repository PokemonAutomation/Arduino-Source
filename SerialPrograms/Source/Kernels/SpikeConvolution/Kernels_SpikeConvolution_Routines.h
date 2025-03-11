/*  Spike Convolution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_SpikeConvolution_Routines_H
#define PokemonAutomation_Kernels_SpikeConvolution_Routines_H

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


template <typename Context, bool addto>
PA_FORCE_INLINE void accumulate_k1(
    typename Context::vtype* out, const float* in, size_t lengthI,
    typename Context::vtype k0
){
    using vtype = typename Context::vtype;
    constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    size_t lc = lengthI / (4 * VECTOR_LENGTH);
    while (lc--){
        vtype out0, out1, out2, out3;
        if (addto){
            out0 = out[0];
            out1 = out[1];
            out2 = out[2];
            out3 = out[3];
            Context::accumulate(out0, k0, in + 0 * VECTOR_LENGTH);
            Context::accumulate(out1, k0, in + 1 * VECTOR_LENGTH);
            Context::accumulate(out2, k0, in + 2 * VECTOR_LENGTH);
            Context::accumulate(out3, k0, in + 3 * VECTOR_LENGTH);
        }else{
            out0 = Context::multiply(k0, in + 0 * VECTOR_LENGTH);
            out1 = Context::multiply(k0, in + 1 * VECTOR_LENGTH);
            out2 = Context::multiply(k0, in + 2 * VECTOR_LENGTH);
            out3 = Context::multiply(k0, in + 3 * VECTOR_LENGTH);
        }
        out[0] = out0;
        out[1] = out1;
        out[2] = out2;
        out[3] = out3;
        in += 4 * VECTOR_LENGTH;
        out += 4;
    }
    lengthI %= 4 * VECTOR_LENGTH;
    while (lengthI >= VECTOR_LENGTH){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate(out0, k0, in + 0 * VECTOR_LENGTH);
        }else{
            out0 = Context::multiply(k0, in + 0 * VECTOR_LENGTH);
        }
        out[0] = out0;
        in += VECTOR_LENGTH;
        out += 1;
        lengthI -= VECTOR_LENGTH;
    }
    if (lengthI){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate_partial(out0, k0, in, lengthI);
        }else{
            out0 = Context::multiply_partial(k0, in, lengthI);
        }
//        Context::store_partial((float*)out, out0, lengthI);
        out[0] = out0;
    }
}
template <typename Context, bool addto>
PA_FORCE_INLINE void accumulate_k2(
    typename Context::vtype* out, const float* in, size_t lengthI,
    typename Context::vtype k0,
    typename Context::vtype k1
){
    using vtype = typename Context::vtype;
    constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    size_t lc = lengthI / (4 * VECTOR_LENGTH);
    while (lc--){
        vtype out0, out1, out2, out3;
        if (addto){
            out0 = out[0];
            out1 = out[1];
            out2 = out[2];
            out3 = out[3];
            Context::accumulate(out0, k0, in + 0 * VECTOR_LENGTH);
            Context::accumulate(out1, k0, in + 1 * VECTOR_LENGTH);
            Context::accumulate(out2, k0, in + 2 * VECTOR_LENGTH);
            Context::accumulate(out3, k0, in + 3 * VECTOR_LENGTH);
        }else{
            out0 = Context::multiply(k0, in + 0 * VECTOR_LENGTH);
            out1 = Context::multiply(k0, in + 1 * VECTOR_LENGTH);
            out2 = Context::multiply(k0, in + 2 * VECTOR_LENGTH);
            out3 = Context::multiply(k0, in + 3 * VECTOR_LENGTH);
        }
        Context::accumulate(out0, k1, in + 0 * VECTOR_LENGTH + 1);
        Context::accumulate(out1, k1, in + 1 * VECTOR_LENGTH + 1);
        Context::accumulate(out2, k1, in + 2 * VECTOR_LENGTH + 1);
        Context::accumulate(out3, k1, in + 3 * VECTOR_LENGTH + 1);
        out[0] = out0;
        out[1] = out1;
        out[2] = out2;
        out[3] = out3;
        in += 4 * VECTOR_LENGTH;
        out += 4;
    }
    lengthI %= 4 * VECTOR_LENGTH;
    while (lengthI >= VECTOR_LENGTH){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate(out0, k0, in);
        }else{
            out0 = Context::multiply(k0, in);
        }
        Context::accumulate(out0, k1, in + 1);
        out[0] = out0;
        in += VECTOR_LENGTH;
        out += 1;
        lengthI -= VECTOR_LENGTH;
    }
    if (lengthI){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate_partial(out0, k0, in, lengthI);
        }else{
            out0 = Context::multiply_partial(k0, in, lengthI);
        }
        Context::accumulate_partial(out0, k1, in + 1, lengthI);
//        Context::store_partial((float*)out, out0, lengthI);
        out[0] = out0;
    }
}
template <typename Context, bool addto>
PA_FORCE_INLINE void accumulate_k3(
    typename Context::vtype* out, const float* in, size_t lengthI,
    typename Context::vtype k0,
    typename Context::vtype k1,
    typename Context::vtype k2
){
    using vtype = typename Context::vtype;
    constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    size_t lc = lengthI / (4 * VECTOR_LENGTH);
    while (lc--){
        vtype out0, out1, out2, out3;
        if (addto){
            out0 = out[0];
            out1 = out[1];
            out2 = out[2];
            out3 = out[3];
            Context::accumulate(out0, k0, in + 0 * VECTOR_LENGTH);
            Context::accumulate(out1, k0, in + 1 * VECTOR_LENGTH);
            Context::accumulate(out2, k0, in + 2 * VECTOR_LENGTH);
            Context::accumulate(out3, k0, in + 3 * VECTOR_LENGTH);
        }else{
            out0 = Context::multiply(k0, in + 0 * VECTOR_LENGTH);
            out1 = Context::multiply(k0, in + 1 * VECTOR_LENGTH);
            out2 = Context::multiply(k0, in + 2 * VECTOR_LENGTH);
            out3 = Context::multiply(k0, in + 3 * VECTOR_LENGTH);
        }
        Context::accumulate(out0, k1, in + 0 * VECTOR_LENGTH + 1);
        Context::accumulate(out1, k1, in + 1 * VECTOR_LENGTH + 1);
        Context::accumulate(out2, k1, in + 2 * VECTOR_LENGTH + 1);
        Context::accumulate(out3, k1, in + 3 * VECTOR_LENGTH + 1);
        Context::accumulate(out0, k2, in + 0 * VECTOR_LENGTH + 2);
        Context::accumulate(out1, k2, in + 1 * VECTOR_LENGTH + 2);
        Context::accumulate(out2, k2, in + 2 * VECTOR_LENGTH + 2);
        Context::accumulate(out3, k2, in + 3 * VECTOR_LENGTH + 2);
        out[0] = out0;
        out[1] = out1;
        out[2] = out2;
        out[3] = out3;
        in += 4 * VECTOR_LENGTH;
        out += 4;
    }
    lengthI %= 4 * VECTOR_LENGTH;
    while (lengthI >= VECTOR_LENGTH){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate(out0, k0, in);
        }else{
            out0 = Context::multiply(k0, in);
        }
        Context::accumulate(out0, k1, in + 1);
        Context::accumulate(out0, k2, in + 2);
        out[0] = out0;
        in += VECTOR_LENGTH;
        out += 1;
        lengthI -= VECTOR_LENGTH;
    }
    if (lengthI){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate_partial(out0, k0, in, lengthI);
        }else{
            out0 = Context::multiply_partial(k0, in, lengthI);
        }
        Context::accumulate_partial(out0, k1, in + 1, lengthI);
        Context::accumulate_partial(out0, k2, in + 2, lengthI);
//        Context::store_partial((float*)out, out0, lengthI);
        out[0] = out0;
    }
}
template <typename Context, bool addto>
PA_FORCE_INLINE void accumulate_k4(
    typename Context::vtype* out, const float* in, size_t lengthI,
    typename Context::vtype k0,
    typename Context::vtype k1,
    typename Context::vtype k2,
    typename Context::vtype k3
){
    using vtype = typename Context::vtype;
    constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    size_t lc = lengthI / (4 * VECTOR_LENGTH);
    while (lc--){
        vtype out0, out1, out2, out3;
        if (addto){
            out0 = out[0];
            out1 = out[1];
            out2 = out[2];
            out3 = out[3];
            Context::accumulate(out0, k0, in + 0 * VECTOR_LENGTH);
            Context::accumulate(out1, k0, in + 1 * VECTOR_LENGTH);
            Context::accumulate(out2, k0, in + 2 * VECTOR_LENGTH);
            Context::accumulate(out3, k0, in + 3 * VECTOR_LENGTH);
        }else{
            out0 = Context::multiply(k0, in + 0 * VECTOR_LENGTH);
            out1 = Context::multiply(k0, in + 1 * VECTOR_LENGTH);
            out2 = Context::multiply(k0, in + 2 * VECTOR_LENGTH);
            out3 = Context::multiply(k0, in + 3 * VECTOR_LENGTH);
        }
        Context::accumulate(out0, k1, in + 0 * VECTOR_LENGTH + 1);
        Context::accumulate(out1, k1, in + 1 * VECTOR_LENGTH + 1);
        Context::accumulate(out2, k1, in + 2 * VECTOR_LENGTH + 1);
        Context::accumulate(out3, k1, in + 3 * VECTOR_LENGTH + 1);
        Context::accumulate(out0, k2, in + 0 * VECTOR_LENGTH + 2);
        Context::accumulate(out1, k2, in + 1 * VECTOR_LENGTH + 2);
        Context::accumulate(out2, k2, in + 2 * VECTOR_LENGTH + 2);
        Context::accumulate(out3, k2, in + 3 * VECTOR_LENGTH + 2);
        Context::accumulate(out0, k3, in + 0 * VECTOR_LENGTH + 3);
        Context::accumulate(out1, k3, in + 1 * VECTOR_LENGTH + 3);
        Context::accumulate(out2, k3, in + 2 * VECTOR_LENGTH + 3);
        Context::accumulate(out3, k3, in + 3 * VECTOR_LENGTH + 3);
        out[0] = out0;
        out[1] = out1;
        out[2] = out2;
        out[3] = out3;
        in += 4 * VECTOR_LENGTH;
        out += 4;
    }
    lengthI %= 4 * VECTOR_LENGTH;
    while (lengthI >= VECTOR_LENGTH){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate(out0, k0, in);
        }else{
            out0 = Context::multiply(k0, in);
        }
        Context::accumulate(out0, k1, in + 1);
        Context::accumulate(out0, k2, in + 2);
        Context::accumulate(out0, k3, in + 3);
        out[0] = out0;
        in += VECTOR_LENGTH;
        out += 1;
        lengthI -= VECTOR_LENGTH;
    }
    if (lengthI){
        vtype out0;
        if (addto){
            out0 = out[0];
            Context::accumulate_partial(out0, k0, in, lengthI);
        }else{
            out0 = Context::multiply_partial(k0, in, lengthI);
        }
        Context::accumulate_partial(out0, k1, in + 1, lengthI);
        Context::accumulate_partial(out0, k2, in + 2, lengthI);
        Context::accumulate_partial(out0, k3, in + 3, lengthI);
//        Context::store_partial((float*)out, out0, lengthI);
        out[0] = out0;
    }
}



//  Compute the Spike Kernel
//      "out" is aligned to "PA_ALIGNMENT" bytes.
//      lengthI >= lengthK
//      "in"  is valid for lengthI
//      "out" is valid for (lengthI - lengthK + 1) rounded up to PA_ALIGNMENT/sizeof(float)
template <typename Context>
PA_FORCE_INLINE void compute_spike_kernel(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
//    constexpr size_t FLOAT_ALIGNMENT = PA_ALIGNMENT / sizeof(float);
    using vtype = typename Context::vtype;
//    constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    if ((size_t)out % PA_ALIGNMENT){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "\"out\" is misaligned.");
    }
//    if (lengthI % FLOAT_ALIGNMENT){
//        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "lengthI is misaligned.");
//    }
    if (lengthI < lengthK){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "lengthK < lengthI");
    }

    lengthI = lengthI - lengthK + 1;
//    size_t lengthO = (lengthI + VECTOR_LENGTH - 1) / VECTOR_LENGTH;
//    cout << "lengthI = " << lengthI << endl;
//    cout << "lengthO = " << lengthO << endl;

    bool first = true;

    while (lengthK >= 4){
        vtype k0 = Context::broadcast(kernel[0]);
        vtype k1 = Context::broadcast(kernel[1]);
        vtype k2 = Context::broadcast(kernel[2]);
        vtype k3 = Context::broadcast(kernel[3]);
        if (first){
            accumulate_k4<Context, false>((vtype*)out, in, lengthI, k0, k1, k2, k3);
            first = false;
        }else{
            accumulate_k4<Context, true>((vtype*)out, in, lengthI, k0, k1, k2, k3);
        }
        in += 4;
        kernel += 4;
        lengthK -= 4;
    }
    if (lengthK == 3){
        vtype k0 = Context::broadcast(kernel[0]);
        vtype k1 = Context::broadcast(kernel[1]);
        vtype k2 = Context::broadcast(kernel[2]);
        if (first){
            accumulate_k3<Context, false>((vtype*)out, in, lengthI, k0, k1, k2);
        }else{
            accumulate_k3<Context, true>((vtype*)out, in, lengthI, k0, k1, k2);
        }
        return;
    }
    if (lengthK == 2){
        vtype k0 = Context::broadcast(kernel[0]);
        vtype k1 = Context::broadcast(kernel[1]);
        if (first){
            accumulate_k2<Context, false>((vtype*)out, in, lengthI, k0, k1);
        }else{
            accumulate_k2<Context, true>((vtype*)out, in, lengthI, k0, k1);
        }
        return;
    }
    if (lengthK == 1){
        vtype k0 = Context::broadcast(kernel[0]);
        if (first){
            accumulate_k1<Context, false>((vtype*)out, in, lengthI, k0);
        }else{
            accumulate_k1<Context, true>((vtype*)out, in, lengthI, k0);
        }
        return;
    }
}



}
}
}
#endif
