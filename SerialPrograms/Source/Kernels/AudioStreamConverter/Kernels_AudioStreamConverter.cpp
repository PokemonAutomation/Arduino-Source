/*  Audio Stream Converter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioStreamConverter.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



template <typename ExternalType>
void convert_from_Default(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_Default(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_Default(UnitTestDatabase& database);



template <typename ExternalType>
void convert_from_x64_SSE41(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_x64_SSE41(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_x64_SSE41(UnitTestDatabase& database);



template <typename ExternalType>
void convert_from_x64_AVX2(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_x64_AVX2(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_x64_AVX2(UnitTestDatabase& database);



template <typename ExternalType>
void convert_from_x64_AVX512(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_x64_AVX512(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_x64_AVX512(UnitTestDatabase& database);





template <typename ExternalType>
void convert_from(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        convert_from_x64_AVX512(native, ext, samples);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_from_x64_AVX2(native, ext, samples);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_from_x64_SSE41(native, ext, samples);
        return;
    }
#endif
    convert_from_Default(native, ext, samples);
    return;
}
template <typename ExternalType>
void convert_to(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        convert_to_x64_AVX512(native, ext, samples, multiplier);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_to_x64_AVX2(native, ext, samples, multiplier);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_to_x64_SSE41(native, ext, samples, multiplier);
        return;
    }
#endif
    convert_to_Default(native, ext, samples, multiplier);
    return;
}



void add_tests(UnitTestDatabase& database){
    add_tests_Default(database);
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x64_SSE41(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x64_AVX2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_x64_AVX512(database);
    }
#endif
}



}
}
}
