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
using ConvertFrom = void (*)(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples, float multiplier);
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
        ConvertTo<Type> convert_to,
        float from_multiplier = 1.0f,
        float to_multiplier = 1.0f
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

public:
    static std::string typestr();

private:
    static std::string make_name(
        const std::string& arch_str,
        size_t samples,
        float from_multiplier,
        float to_multiplier
    );
    size_t m_samples;
    ConvertFrom<Type> m_convert_from;
    ConvertTo<Type> m_convert_to;
    float m_from_multiplier;
    float m_to_multiplier;
};




//  Compares a `to` kernel against the Default (reference) implementation on
//  random float inputs.  The round-trip tests only feed grid-aligned values
//  (the output of `from`), which cannot expose differences in rounding or
//  clamping.
template <typename Type>
class ToArbitraryTest : public UnitTest{
public:
    ToArbitraryTest(
        const std::string& arch_str,
        size_t samples,
        ConvertTo<Type> convert_to
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;

private:
    size_t m_samples;
    ConvertTo<Type> m_convert_to;
};




}
}
}
#endif
