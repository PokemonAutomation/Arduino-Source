/*  Audio Channel Mixer (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
//#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioChannelMixer_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelMixer{



void mix_channels_Default(
    c_ptr<float> matrix, size_t samples,
    size_t out_channels, size_t out_stride, r_ptr<float> out,
    size_t in_channels, size_t in_stride, c_ptr<float> in
){
    size_t combined = (in_channels << 16) | out_channels;
    switch (combined){
    case 0x00010001:
        mix_channels_Default<1, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020001:
        mix_channels_Default<2, 1>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00010002:
        mix_channels_Default<1, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    case 0x00020002:
        mix_channels_Default<2, 2>(matrix, samples, out_stride, out, in_stride, in);
        return;
    default:
        mix_channels_GenericDefault(
            matrix, samples,
            out_channels, out_stride, out,
            in_channels, in_stride, in
        );
    }
}





}
}
}
