/*  ABS FFT (x86 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include "Kernels_AbsFFT_Arch_x86_AVX2.h"
#include "Kernels_AbsFFT_BaseTransform_x86_AVX2.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



TwiddleTable<Context_x86_AVX2>& global_table_x86_AVX2(){
    static TwiddleTable<Context_x86_AVX2> table(14);
    return table;
}
void fft_abs_x86_AVX2(int k, float* abs, float* real){
    TwiddleTable<Context_x86_AVX2>& table = global_table_x86_AVX2();
    table.ensure(k);
    fft_abs(table, k, abs, real);
}



}
}
}
#endif
