/*  Audio Channel Interleave (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelInterleave_Default_H
#define PokemonAutomation_Kernels_AudioChannelInterleave_Default_H

#include "Common/Compiler.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioChannelInterleave{



void channel_deinterleave_Default(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
);
void channel_interleave_Default(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
);



PA_FORCE_INLINE void channel_deinterleave_GenericDefault(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            strided_stream[c*stride] = interleaved_stream[c];
        }
        strided_stream++;
        interleaved_stream += channels;
    }
}
PA_FORCE_INLINE void channel_interleave_GenericDefault(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    for (size_t s = 0; s < samples; s++){
        for (size_t c = 0; c < channels; c++){
            interleaved_stream[c] = strided_stream[c*stride];
        }
        strided_stream++;
        interleaved_stream += channels;
    }
}


void add_tests_Default(UnitTestDatabase& database);


}
}
}
#endif
