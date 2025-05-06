/*  Partial Word Access (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_PartialWordAccess_x64_AVX2_H
#define PokemonAutomation_Kernels_PartialWordAccess_x64_AVX2_H

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


class PartialWordAccess32_x64_AVX2{
public:
    PA_FORCE_INLINE PartialWordAccess32_x64_AVX2(size_t words){
        m_mask = _mm256_cmpgt_epi32(
            _mm256_set1_epi32((uint32_t)words),
            _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)
        );
    }

    PA_FORCE_INLINE __m256i mask() const{
        return m_mask;
    }
    PA_FORCE_INLINE __m256i load_i32(const void* ptr) const{
        return _mm256_maskload_epi32((const int*)ptr, m_mask);
    }
    PA_FORCE_INLINE __m256 load_f32(const void* ptr) const{
        return _mm256_maskload_ps((const float*)ptr, m_mask);
    }
    PA_FORCE_INLINE void store(const void* ptr, __m256i x) const{
        _mm256_maskstore_epi32((int*)ptr, m_mask, x);
    }

private:
    __m256i m_mask;
};



}
}
#endif
