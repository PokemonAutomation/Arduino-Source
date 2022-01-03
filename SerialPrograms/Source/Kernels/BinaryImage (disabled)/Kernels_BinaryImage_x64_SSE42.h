/*  Binary Image (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_x64_SSE42_H
#define PokemonAutomation_Kernels_BinaryImage_x64_SSE42_H

#include <smmintrin.h>
#include "Kernels_BinaryImageBase.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryImageContext_x64_SSE42{
    using vtype = __m128i;
};


class BinaryImage_x64_SSE42 : public BinaryImageBase<BinaryImageContext_x64_SSE42>{
    using Word = __m128i;

public:
    using BinaryImageBase::BinaryImageBase;

    void set_zero(){
        size_t lc = m_data.size();
        Word* ptr = m_data.data();
        do{
            ptr[0] = _mm_setzero_si128();
            ptr++;
        }while (--lc);
    }
};



}
}
#endif
