/*  Audio Channel Interleave (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioChannelInterleave_TestInfra.h"
#include "Kernels_AudioChannelInterleave_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioChannelInterleave{



void channel_deinterleave_Default(
    size_t channels, size_t samples,
    size_t stride, r_ptr<float> strided_stream,
    c_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 1:
        memcpy(strided_stream, interleaved_stream, samples * sizeof(float));
        break;
    case 2:
        channel_deinterleave_GenericDefault(2, samples, stride, strided_stream, interleaved_stream);
        break;
    case 3:
        channel_deinterleave_GenericDefault(3, samples, stride, strided_stream, interleaved_stream);
        break;
    case 4:
        channel_deinterleave_GenericDefault(4, samples, stride, strided_stream, interleaved_stream);
        break;
    case 5:
        channel_deinterleave_GenericDefault(5, samples, stride, strided_stream, interleaved_stream);
        break;
    case 6:
        channel_deinterleave_GenericDefault(6, samples, stride, strided_stream, interleaved_stream);
        break;
    case 7:
        channel_deinterleave_GenericDefault(7, samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_deinterleave_GenericDefault(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}
void channel_interleave_Default(
    size_t channels, size_t samples,
    size_t stride, c_ptr<float> strided_stream,
    r_ptr<float> interleaved_stream
){
    //  We pull out the common cases and force-inline them so the inner loop can
    //  be unrolled.
    switch (channels){
    case 1:
        memcpy(interleaved_stream, strided_stream, samples * sizeof(float));
        break;
    case 2:
        channel_interleave_GenericDefault(2, samples, stride, strided_stream, interleaved_stream);
        break;
    case 3:
        channel_interleave_GenericDefault(3, samples, stride, strided_stream, interleaved_stream);
        break;
    case 4:
        channel_interleave_GenericDefault(4, samples, stride, strided_stream, interleaved_stream);
        break;
    case 5:
        channel_interleave_GenericDefault(5, samples, stride, strided_stream, interleaved_stream);
        break;
    case 6:
        channel_interleave_GenericDefault(6, samples, stride, strided_stream, interleaved_stream);
        break;
    case 7:
        channel_interleave_GenericDefault(7, samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        channel_interleave_GenericDefault(channels, samples, stride, strided_stream, interleaved_stream);
        break;
    }
}


class Test_Default : public Test{
public:
    Test_Default(size_t channels, size_t samples)
        : Test(
            "Default",
            channels, samples,
            channel_deinterleave_Default,
            channel_interleave_Default
        )
    {}
};
void add_tests_Default(UnitTestDatabase& database){
    for (size_t c = 1; c <= 8; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_Default>(c, samples);
        }
    }
}




}
}
}
