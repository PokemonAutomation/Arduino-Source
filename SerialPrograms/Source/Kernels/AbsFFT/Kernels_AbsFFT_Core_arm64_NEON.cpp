/*  ABS FFT (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AbsFFT_Arch_arm64_NEON.h"
#include "Kernels_AbsFFT_BaseTransform_arm64_NEON.h"
#include "Kernels_AbsFFT_TwiddleTable.tpp"
#include "Kernels_AbsFFT_FullTransform.tpp"
#include "Kernels_AbsFFT_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



TwiddleTable<Context_arm64_NEON>& global_table_arm64_NEON(){
    static TwiddleTable<Context_arm64_NEON> table(14);
    return table;
}
void fft_abs_arm64_NEON(int k, float* abs, float* real, const float* weights){
    TwiddleTable<Context_arm64_NEON>& table = global_table_arm64_NEON();
    table.ensure(k);
    if (weights == nullptr){
        fft_abs<Context_arm64_NEON, false>(table, k, abs, real, nullptr);
    }else{
        fft_abs<Context_arm64_NEON, true>(table, k, abs, real, weights);
    }
}



void fft_abs_Default(int k, float* abs, float* real, const float* weights);

class Test_arm64_NEON : public Test{
public:
    Test_arm64_NEON(int k, bool use_weights)
        : Test(
            "arm64_NEON",
            k, use_weights,
            fft_abs_Default,
            fft_abs_arm64_NEON
        )
    {}
};
void add_tests_arm64_NEON(UnitTestDatabase& database){
    for (int k = 1; k <= 16; k++){
        database.add<Test_arm64_NEON>(k, false);
        database.add<Test_arm64_NEON>(k, true);
    }
}



}
}
}
#endif
