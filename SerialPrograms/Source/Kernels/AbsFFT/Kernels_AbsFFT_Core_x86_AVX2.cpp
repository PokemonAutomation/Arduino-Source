/*  ABS FFT (x86 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AbsFFT_Arch_x86_AVX2.h"
#include "Kernels_AbsFFT_BaseTransform_x86_AVX2.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"
#include "Kernels_AbsFFT_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



TwiddleTable<Context_x86_AVX2>& global_table_x86_AVX2(){
    static TwiddleTable<Context_x86_AVX2> table(14);
    return table;
}
void fft_abs_x86_AVX2(int k, float* abs, float* real, const float* weights){
    TwiddleTable<Context_x86_AVX2>& table = global_table_x86_AVX2();
    table.ensure(k);
    if (weights == nullptr){
        fft_abs<Context_x86_AVX2, false>(table, k, abs, real, nullptr);
    }else{
        fft_abs<Context_x86_AVX2, true>(table, k, abs, real, weights);
    }
}



void fft_abs_Default(int k, float* abs, float* real, const float* weights);

class Test_x86_AVX2 : public Test{
public:
    Test_x86_AVX2(int k, bool use_weights)
        : Test(
            "x86_AVX2",
            k, use_weights,
            fft_abs_Default,
            fft_abs_x86_AVX2
        )
    {}
};
void add_tests_x86_AVX2(UnitTestDatabase& database){
    for (int k = 1; k <= 16; k++){
        database.add<Test_x86_AVX2>(k, false);
        database.add<Test_x86_AVX2>(k, true);
    }
}



}
}
}
#endif
