/*  Audio Channel Mixer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioChannelMixer_H
#define PokemonAutomation_Kernels_AudioChannelMixer_H

#include "Common/Compiler.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace Kernels{
namespace AudioChannelMixer{


void mix_channels(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
);


void add_tests(UnitTestDatabase& database);


}
}
}
#endif
