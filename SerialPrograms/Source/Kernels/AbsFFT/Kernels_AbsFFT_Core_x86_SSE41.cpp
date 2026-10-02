/*  ABS FFT (x86 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include "Kernels_AbsFFT_Arch_x86_SSE41.h"
#include "Kernels_AbsFFT_BaseTransform_x86_SSE41.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



TwiddleTable<Context_x86_SSE41>& global_table_x86_SSE41(){
    static TwiddleTable<Context_x86_SSE41> table(14);
    return table;
}
void fft_abs_x86_SSE41(int k, float* abs, float* real, const float* weights){
    TwiddleTable<Context_x86_SSE41>& table = global_table_x86_SSE41();
    table.ensure(k);
    if (weights == nullptr){
        fft_abs<Context_x86_SSE41, false>(table, k, abs, real, nullptr);
    }else{
        fft_abs<Context_x86_SSE41, true>(table, k, abs, real, weights);
    }
}



}
}
}
#endif
