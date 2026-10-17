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
    //  Raw -> Working -> Raw round-trip, with reciprocal multipliers on
    //  the two nodes (exact in binary) to exercise the gain path.
    const float from_multipliers[] = { 1.0f, 0.5f };
    for (size_t channels = 1; channels <= 4; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            for (float from_multiplier : from_multipliers){
                database.add<RoundTripTest>(SampleFormat::UINT8, channels, samples, from_multiplier);
                database.add<RoundTripTest>(SampleFormat::SINT16, channels, samples, from_multiplier);
                database.add<RoundTripTest>(SampleFormat::SINT32, channels, samples, from_multiplier);
                database.add<RoundTripTest>(SampleFormat::FLOAT32, channels, samples, from_multiplier);
            }
        }
    }

    //  ChannelMixer with an identity weight matrix.
    for (size_t channels = 1; channels <= 4; channels++){
        database.add<MixerIdentityTest>(channels, 1000);
    }

    //  1 -> 2 -> 1 averaging round-trip.
    database.add<MixerRoundTripTest>(1000);
}




}
}
}
