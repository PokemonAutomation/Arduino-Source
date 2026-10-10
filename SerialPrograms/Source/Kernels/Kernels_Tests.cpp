/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_Tests.h"
#include "AudioStreamConverter/Kernels_AudioStreamConverter_Kernels_Default.h"
#include "BinaryMatrix/Kernels_BinaryMatrix_Tests.h"
#include "ImageFilters/Kernels_ImageFilter_Tests.h"
#include "ImageScaleBrightness/Kernels_ImageScaleBrightness_Tests.h"
#include "Waterfill/Kernels_Waterfill_Tests.h"

#ifdef PA_AutoDispatch_x64_13_Haswell
#include "AudioStreamConverter/Kernels_AudioStreamConverter_Kernels_x64_AVX2.h"
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
#include "AudioStreamConverter/Kernels_AudioStreamConverter_Kernels_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{



void add_tests(UnitTestDatabase& database){
    AudioStreamConverter::add_tests_Default(database);
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        AudioStreamConverter::add_tests_x64_AVX2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        AudioStreamConverter::add_tests_x64_AVX512(database);
    }
#endif
    add_tests_BinaryMatrix(database);
    add_tests_ImageFilters(database);
    add_tests_ImageScaleBrightness(database);
    add_tests_Waterfill(database);
}



}
}
