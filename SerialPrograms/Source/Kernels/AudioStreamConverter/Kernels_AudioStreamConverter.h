/*  Audio Stream Converter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioStreamConverter_H
#define PokemonAutomation_Kernels_AudioStreamConverter_H

#include <cstddef>
#include "Common/Compiler.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioStreamConverter{



template <typename ExternalType>
void convert_from(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);



void add_tests(UnitTestDatabase& database);




}
}
}
#endif
