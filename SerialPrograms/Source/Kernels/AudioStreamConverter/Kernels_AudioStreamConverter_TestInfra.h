/*  Audio Stream Converter Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioStreamConverter_TestInfra_H
#define PokemonAutomation_Kernels_AudioStreamConverter_TestInfra_H

#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{





template <typename ExternalType>
using ConvertFrom = void (*)(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
using ConvertTo = void (*)(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);






//
//  Warning, this file is compiled in multiple ISA contexts.
//  We cannot define the functions here since they will be compiled differently.
//  If a higher (AVX512) version wins out in the linker, all versions will use
//  it and crash on older CPUs.
//
template <typename Type>
class Test : public UnitTest{
public:
    Test(
        const std::string& arch_str,
        size_t samples,
        ConvertFrom<Type> convert_from,
        ConvertTo<Type> convert_to
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

    static std::string typestr();

private:
    size_t m_samples;
    ConvertFrom<Type> m_convert_from;
    ConvertTo<Type> m_convert_to;
};




}
}
}
#endif
