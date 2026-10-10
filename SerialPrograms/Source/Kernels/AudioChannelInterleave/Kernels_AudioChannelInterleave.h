/*  Audio Channel Interleave
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelInterleave_H
#define PokemonAutomation_Kernels_AudioChannelInterleave_H

#include <cstddef>
#include "Common/Compiler.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioChannelInterleave{



void channel_deinterleave(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
void channel_interleave(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);


void add_tests(UnitTestDatabase& database);




}
}
}
#endif
