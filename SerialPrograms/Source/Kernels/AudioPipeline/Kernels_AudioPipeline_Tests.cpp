/*  Audio Pipeline Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioPipeline.h"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_ChannelMixer_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




void add_tests(UnitTestDatabase& database){
    //  Raw -> Working -> Raw round-trip.
    for (size_t channels = 1; channels <= 4; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<RoundTripTest>(SampleFormat::UINT8, channels, samples);
            database.add<RoundTripTest>(SampleFormat::SINT16, channels, samples);
            database.add<RoundTripTest>(SampleFormat::SINT32, channels, samples);
            database.add<RoundTripTest>(SampleFormat::FLOAT32, channels, samples);
        }
    }

    //  ChannelMixer with an identity weight matrix.
    for (size_t channels = 1; channels <= 4; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<MixerIdentityTest>(channels, samples);
        }
    }

    //  1 -> 2 -> 1 averaging round-trip.
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        database.add<MixerRoundTripTest>(samples);
    }
}




}
}
}
