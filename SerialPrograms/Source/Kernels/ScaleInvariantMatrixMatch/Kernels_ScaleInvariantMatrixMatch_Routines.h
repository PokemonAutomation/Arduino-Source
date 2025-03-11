/*  Scale Invariant Matrix Match Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_Routines_H
#define PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_Routines_H

#include <stddef.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



template <typename Context>
struct SumATA2{
    using vtype = typename Context::vtype;
    static constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    vtype sum_AT = Context::vzero();
    vtype sum_A2 = Context::vzero();

    PA_FORCE_INLINE float scale() const{
        return Context::vreduce(sum_AT) / Context::vreduce(sum_A2);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        vtype sum_as0 = Context::vzero();
        vtype sum_as1 = Context::vzero();
        vtype sum_as2 = Context::vzero();
        vtype sum_as3 = Context::vzero();
        vtype sum_at0 = Context::vzero();
        vtype sum_at1 = Context::vzero();
        vtype sum_at2 = Context::vzero();
        vtype sum_at3 = Context::vzero();

        if (VECTOR_LENGTH > 1){
            size_t align = (size_t)T % (VECTOR_LENGTH * sizeof(float));
            if (align){
                align /= sizeof(float);
                A -= align;
                T -= align;

                vtype a0, t0;
                Context::load2_partial_back(align, a0, A, t0, T);
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_at0 = Context::vpma(a0, t0, sum_at0);

                A += VECTOR_LENGTH;
                T += VECTOR_LENGTH;
                length -= VECTOR_LENGTH - align;
            }
        }

        const vtype* ptrA = (const vtype*)A;
        const vtype* ptrT = (const vtype*)T;

        size_t lc = length / (4 * VECTOR_LENGTH);
        if (lc){
            do{
                vtype a0 = ptrA[0];
                vtype a1 = ptrA[1];
                vtype a2 = ptrA[2];
                vtype a3 = ptrA[3];
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_as1 = Context::vpma(a1, a1, sum_as1);
                sum_as2 = Context::vpma(a2, a2, sum_as2);
                sum_as3 = Context::vpma(a3, a3, sum_as3);
                sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
                sum_at1 = Context::vpma(a1, ptrT[1], sum_at1);
                sum_at2 = Context::vpma(a2, ptrT[2], sum_at2);
                sum_at3 = Context::vpma(a3, ptrT[3], sum_at3);
                ptrA += 4;
                ptrT += 4;
            }while (--lc);
            sum_as0 = Context::vadd(sum_as0, sum_as1);
            sum_at0 = Context::vadd(sum_at0, sum_at1);
            sum_as2 = Context::vadd(sum_as2, sum_as3);
            sum_at2 = Context::vadd(sum_at2, sum_at3);
            sum_as0 = Context::vadd(sum_as0, sum_as2);
            sum_at0 = Context::vadd(sum_at0, sum_at2);
        }

        length %= 4 * VECTOR_LENGTH;
        while (length >= VECTOR_LENGTH){
            vtype a0 = ptrA[0];
            sum_as0 = Context::vpma(a0, a0, sum_as0);
            sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
            ptrA += 1;
            ptrT += 1;
            length -= VECTOR_LENGTH;
        }
        if (VECTOR_LENGTH > 1 && length){
            vtype a0, t0;
            Context::load2_partial_front(length, a0, ptrT, t0, ptrA);
            sum_at0 = Context::vpma(a0, t0, sum_at0);
            sum_as0 = Context::vpma(a0, a0, sum_as0);
        }

        sum_A2 = Context::vadd(sum_A2, sum_as0);
        sum_AT = Context::vadd(sum_AT, sum_at0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW, const float* W){
        vtype sum_as0 = Context::vzero();
        vtype sum_as1 = Context::vzero();
        vtype sum_as2 = Context::vzero();
        vtype sum_as3 = Context::vzero();
        vtype sum_at0 = Context::vzero();
        vtype sum_at1 = Context::vzero();
        vtype sum_at2 = Context::vzero();
        vtype sum_at3 = Context::vzero();

        if (VECTOR_LENGTH > 1){
            size_t align = (size_t)TW % (VECTOR_LENGTH * sizeof(float));
            if (align){
                align /= sizeof(float);
                A -= align;
                TW -= align;
                W -= align;

                vtype a0, t0, w0;
                Context::load3_partial_back(align, a0, A, t0, TW, w0, W);
                a0 = Context::vmul(a0, w0);
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_at0 = Context::vpma(a0, t0, sum_at0);

                A += VECTOR_LENGTH;
                TW += VECTOR_LENGTH;
                W += VECTOR_LENGTH;
                length -= VECTOR_LENGTH - align;
            }
        }

        const vtype* ptrA = (const vtype*)A;
        const vtype* ptrT = (const vtype*)TW;
        const vtype* ptrW = (const vtype*)W;

        size_t lc = length / (4 * VECTOR_LENGTH);
        if (lc){
            do{
                vtype a0 = ptrA[0];
                vtype a1 = ptrA[1];
                vtype a2 = ptrA[2];
                vtype a3 = ptrA[3];
                a0 = Context::vmul(a0, ptrW[0]);
                a1 = Context::vmul(a1, ptrW[1]);
                a2 = Context::vmul(a2, ptrW[2]);
                a3 = Context::vmul(a3, ptrW[3]);
                sum_as0 = Context::vpma(a0, a0, sum_as0);
                sum_as1 = Context::vpma(a1, a1, sum_as1);
                sum_as2 = Context::vpma(a2, a2, sum_as2);
                sum_as3 = Context::vpma(a3, a3, sum_as3);
                sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
                sum_at1 = Context::vpma(a1, ptrT[1], sum_at1);
                sum_at2 = Context::vpma(a2, ptrT[2], sum_at2);
                sum_at3 = Context::vpma(a3, ptrT[3], sum_at3);
                ptrA += 4;
                ptrT += 4;
                ptrW += 4;
            }while (--lc);
            sum_as0 = Context::vadd(sum_as0, sum_as1);
            sum_at0 = Context::vadd(sum_at0, sum_at1);
            sum_as2 = Context::vadd(sum_as2, sum_as3);
            sum_at2 = Context::vadd(sum_at2, sum_at3);
            sum_as0 = Context::vadd(sum_as0, sum_as2);
            sum_at0 = Context::vadd(sum_at0, sum_at2);
        }

        length %= 4 * VECTOR_LENGTH;
        while (length >= VECTOR_LENGTH){
            vtype a0 = ptrA[0];
            a0 = Context::vmul(a0, ptrW[0]);
            sum_as0 = Context::vpma(a0, a0, sum_as0);
            sum_at0 = Context::vpma(a0, ptrT[0], sum_at0);
            ptrA += 1;
            ptrT += 1;
            ptrW += 1;
            length -= VECTOR_LENGTH;
        }
        if (length){
            vtype a0, t0, w0;
            Context::load3_partial_front(length, a0, A, t0, TW, w0, W);
            a0 = Context::vmul(a0, w0);
            sum_as0 = Context::vpma(a0, a0, sum_as0);
            sum_at0 = Context::vpma(a0, t0, sum_at0);
        }

        sum_A2 = Context::vadd(sum_A2, sum_as0);
        sum_AT = Context::vadd(sum_AT, sum_at0);
    }
};



template <typename Context>
struct SumError{
    using vtype = typename Context::vtype;
    static constexpr size_t VECTOR_LENGTH = sizeof(vtype) / sizeof(float);

    vtype scale;
    vtype sum = Context::vzero();

    PA_FORCE_INLINE SumError(float p_scale)
        : scale(Context::vset(p_scale))
    {}

    PA_FORCE_INLINE float sum_sqr() const{
        return Context::vreduce(sum);
    }

    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* T){
        vtype sum0 = Context::vzero();
        vtype sum1 = Context::vzero();
        vtype sum2 = Context::vzero();
        vtype sum3 = Context::vzero();

        if (VECTOR_LENGTH > 1){
            size_t align = (size_t)T % (VECTOR_LENGTH * sizeof(float));
            if (align){
                align /= sizeof(float);
                A -= align;
                T -= align;

                vtype a0, t0;
                Context::load2_partial_back(align, a0, A, t0, T);
                a0 = Context::vpms(scale, a0, t0);
                sum0 = Context::vpma(a0, a0, sum0);

                A += VECTOR_LENGTH;
                T += VECTOR_LENGTH;
                length -= VECTOR_LENGTH - align;
            }
        }

        const vtype* ptrA = (const vtype*)A;
        const vtype* ptrT = (const vtype*)T;

        size_t lc = length / (4 * VECTOR_LENGTH);
        if (lc){
            do{
                vtype a0 = Context::vpms(scale, ptrA[0], ptrT[0]);
                vtype a1 = Context::vpms(scale, ptrA[1], ptrT[1]);
                vtype a2 = Context::vpms(scale, ptrA[2], ptrT[2]);
                vtype a3 = Context::vpms(scale, ptrA[3], ptrT[3]);
                sum0 = Context::vpma(a0, a0, sum0);
                sum1 = Context::vpma(a1, a1, sum1);
                sum2 = Context::vpma(a2, a2, sum2);
                sum3 = Context::vpma(a3, a3, sum3);
                ptrA += 4;
                ptrT += 4;
            }while (--lc);
            sum0 = Context::vadd(sum0, sum1);
            sum2 = Context::vadd(sum2, sum3);
            sum0 = Context::vadd(sum0, sum2);
        }

        length %= 4 * VECTOR_LENGTH;
        while (length >= VECTOR_LENGTH){
            vtype a0 = Context::vpms(scale, ptrA[0], ptrT[0]);
            sum0 = Context::vpma(a0, a0, sum0);
            ptrA += 1;
            ptrT += 1;
            length -= VECTOR_LENGTH;
        }
        if (length){
            vtype a0, t0;
            Context::load2_partial_front(length, a0, ptrT, t0, ptrA);
            a0 = Context::vpms(scale, a0, t0);
            sum0 = Context::vpma(a0, a0, sum0);
        }

        sum = Context::vadd(sum, sum0);
    }
    PA_FORCE_INLINE void accumulate(size_t length, const float* A, const float* TW, const float* W){
        vtype sum0 = Context::vzero();
        vtype sum1 = Context::vzero();
        vtype sum2 = Context::vzero();
        vtype sum3 = Context::vzero();

        if (VECTOR_LENGTH > 1){
            size_t align = (size_t)TW % (VECTOR_LENGTH * sizeof(float));
            if (align){
                align /= sizeof(float);
                A -= align;
                TW -= align;
                W -= align;

                vtype a0, t0, w0;
                Context::load3_partial_back(align, a0, A, t0, TW, w0, W);
                a0 = Context::vmul(scale, a0);
                a0 = Context::vpms(a0, w0, t0);
                sum0 = Context::vpma(a0, a0, sum0);

                A += VECTOR_LENGTH;
                TW += VECTOR_LENGTH;
                W += VECTOR_LENGTH;
                length -= VECTOR_LENGTH - align;
            }
        }

        const vtype* ptrA = (const vtype*)A;
        const vtype* ptrT = (const vtype*)TW;
        const vtype* ptrW = (const vtype*)W;

        size_t lc = length / (4 * VECTOR_LENGTH);
        if (lc){
            do{
                vtype a0 = Context::vmul(scale, ptrA[0]);
                vtype a1 = Context::vmul(scale, ptrA[1]);
                vtype a2 = Context::vmul(scale, ptrA[2]);
                vtype a3 = Context::vmul(scale, ptrA[3]);
                a0 = Context::vpms(a0, ptrW[0], ptrT[0]);
                a1 = Context::vpms(a1, ptrW[1], ptrT[1]);
                a2 = Context::vpms(a2, ptrW[2], ptrT[2]);
                a3 = Context::vpms(a3, ptrW[3], ptrT[3]);
                sum0 = Context::vpma(a0, a0, sum0);
                sum1 = Context::vpma(a1, a1, sum1);
                sum2 = Context::vpma(a2, a2, sum2);
                sum3 = Context::vpma(a3, a3, sum3);
                ptrA += 4;
                ptrT += 4;
                ptrW += 4;
            }while (--lc);
            sum0 = Context::vadd(sum0, sum1);
            sum2 = Context::vadd(sum2, sum3);
            sum0 = Context::vadd(sum0, sum2);
        }

        length %= 4 * VECTOR_LENGTH;
        while (length >= VECTOR_LENGTH){
            vtype a0 = Context::vmul(scale, ptrA[0]);
            a0 = Context::vpms(a0, ptrW[0], ptrT[0]);
            sum0 = Context::vpma(a0, a0, sum0);
            ptrA += 1;
            ptrT += 1;
            ptrW += 1;
            length -= VECTOR_LENGTH;
        }
        if (length){
            vtype a0, t0, w0;
            Context::load3_partial_front(length, a0, A, t0, TW, w0, W);
            a0 = Context::vmul(scale, a0);
            a0 = Context::vpms(a0, w0, t0);
            sum0 = Context::vpma(a0, a0, sum0);
        }

        sum = Context::vadd(sum, sum0);
    }
};



template <typename SumATA2>
PA_FORCE_INLINE float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    constexpr size_t ALIGNMENT = alignof(typename SumATA2::vtype);
    SumATA2 sum;
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = T[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A and T must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT);
    }
    return sum.scale();
}
template <typename SumATA2>
PA_FORCE_INLINE float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    constexpr size_t ALIGNMENT = alignof(typename SumATA2::vtype);
    SumATA2 sum;
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = TW[r];
        const float* ptrW = W[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT || (size_t)ptrA % ALIGNMENT != (size_t)ptrW % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A, TW2, and W2 must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT, ptrW);
    }
    return sum.scale();
}


template <typename SumError>
PA_FORCE_INLINE float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
    constexpr size_t ALIGNMENT = alignof(typename SumError::vtype);
    SumError sum(scale);
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = T[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A and T must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT);
    }
    return sum.sum_sqr();
}
template <typename SumError>
PA_FORCE_INLINE float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
    constexpr size_t ALIGNMENT = alignof(typename SumError::vtype);
    SumError sum(scale);
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = TW[r];
        const float* ptrW = W[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT || (size_t)ptrA % ALIGNMENT != (size_t)ptrW % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A, TW2, and W2 must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT, ptrW);
    }
    return sum.sum_sqr();
}




}
}
}
#endif
