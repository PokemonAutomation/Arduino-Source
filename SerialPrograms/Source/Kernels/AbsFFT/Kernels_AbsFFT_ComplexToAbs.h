/*  ABS FFT Complex to Abs
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_ComplexToAbs_H
#define PokemonAutomation_Kernels_AbsFFT_ComplexToAbs_H

#include "Kernels_AbsFFT_Arch.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
template <typename Context>
struct MiscRoutines{

using vtype = typename Context::vtype;


static PA_FORCE_INLINE void complex_to_abs(vtype* abs, const vtype* data, size_t vlength){
    do{
        abs[0] = Context::abs(data[0], data[1]);
        abs++;
        data += 2;
    }while (--vlength);
}
static PA_FORCE_INLINE void complex_to_abs_swap_odd(vtype* abs, const vtype* data, size_t vlength){
    vtype* absL = abs;
    vtype* absH = abs + vlength;
    const vtype* dataL = data;
    const vtype* dataH = data + 2*vlength;

    if (Context::VECTOR_K == 0){
        size_t lc = vlength / 4;
        do{
            absH -= 2;
            dataH -= 4;

            vtype L0 = Context::abs(dataL[0], dataL[1]);
            vtype L1 = Context::abs(dataL[2], dataL[3]);
            vtype H0 = Context::abs(dataH[0], dataH[1]);
            vtype H1 = Context::abs(dataH[2], dataH[3]);

            absL[0] = L0;
            absL[1] = H1;
            absH[0] = H0;
            absH[1] = L1;

            absL += 2;
            dataL += 4;
        }while (--lc);
    }else{
        size_t lc = vlength / 2;
        do{
            absH -= 1;
            dataH -= 2;

            vtype L0 = Context::abs(dataL[0], dataL[1]);
            vtype H0 = Context::abs(dataH[0], dataH[1]);

            Context::swap_odd(L0, H0);

            absL[0] = L0;
            absH[0] = H0;

            absL += 1;
            dataL += 2;
        }while (--lc);
    }
}


};
}
}
}
#endif
