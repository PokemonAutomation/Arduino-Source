/*  ABS FFT (x86 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Kernels_AbsFFT_Arch_x86_AVX512.h"
#include "Kernels_AbsFFT_BaseTransform_x86_AVX512.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



TwiddleTable<Context_x86_AVX512>& global_table_x86_AVX512(){
    static TwiddleTable<Context_x86_AVX512> table(14);
    return table;
}
void fft_abs_x86_AVX512(int k, float* abs, float* real){
    TwiddleTable<Context_x86_AVX512>& table = global_table_x86_AVX512();
    table.ensure(k);
    fft_abs(table, k, abs, real);
}



}
}
}
#endif
