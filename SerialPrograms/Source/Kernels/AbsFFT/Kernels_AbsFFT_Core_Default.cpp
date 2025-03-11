/*  ABS FFT (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_AbsFFT_Arch_Default.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



template <>
void base_transform<Context_Default>(const TwiddleTable<Context_Default>& table, Context_Default::vtype* T){
    fft_complex_tk_scalar<Context_Default>(table, Context_Default::BASE_COMPLEX_TRANSFORM_K, T);
}



TwiddleTable<Context_Default>& global_table_Default(){
    static TwiddleTable<Context_Default> table(14);
    return table;
}
void fft_abs_Default(int k, float* abs, float* real){
    TwiddleTable<Context_Default>& table = global_table_Default();
    table.ensure(k);
    fft_abs(table, k, abs, real);
}




}
}
}
