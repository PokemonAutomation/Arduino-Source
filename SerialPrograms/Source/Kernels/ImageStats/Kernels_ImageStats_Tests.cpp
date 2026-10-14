/*  Image Stats Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImageStats_Tests.h"

namespace PokemonAutomation{
namespace Kernels{


void add_tests_pixel_sum_sqr_Default(UnitTestDatabase& database);
void add_tests_sum_sqr_deviation_Default(UnitTestDatabase& database);

#ifdef PA_AutoDispatch_x64_08_Nehalem
void add_tests_pixel_sum_sqr_x64_SSE41(UnitTestDatabase& database);
void add_tests_sum_sqr_deviation_x64_SSE41(UnitTestDatabase& database);
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
void add_tests_pixel_sum_sqr_x64_AVX2(UnitTestDatabase& database);
void add_tests_sum_sqr_deviation_x64_AVX2(UnitTestDatabase& database);
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
void add_tests_pixel_sum_sqr_x64_AVX512(UnitTestDatabase& database);
void add_tests_sum_sqr_deviation_x64_AVX512(UnitTestDatabase& database);
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
void add_tests_pixel_sum_sqr_arm64_NEON(UnitTestDatabase& database);
#endif



//
//  Each ISA registers its own set of tests, and every set checks its results
//  against a known (scalar) output, so all ISAs must give identical results.
//
void add_tests_ImageStats(UnitTestDatabase& database){
    add_tests_pixel_sum_sqr_Default(database);
    add_tests_sum_sqr_deviation_Default(database);
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_pixel_sum_sqr_x64_SSE41(database);
        add_tests_sum_sqr_deviation_x64_SSE41(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_pixel_sum_sqr_x64_AVX2(database);
        add_tests_sum_sqr_deviation_x64_AVX2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_pixel_sum_sqr_x64_AVX512(database);
        add_tests_sum_sqr_deviation_x64_AVX512(database);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        add_tests_pixel_sum_sqr_arm64_NEON(database);
    }
#endif
}



}
}
