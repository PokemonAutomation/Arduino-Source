/*  Audio Pipeline Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioPipeline.h"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_ChannelMixer_TestInfra.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter_Kernels_Default.h"
#include "Kernels_RawAudio2WorkingAudio.h"
#include "Kernels_WorkingAudio2RawAudio.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{






namespace{

//  TEMPORARY CI DIAGNOSTIC (remove before merge): reports bit-exact
//  intermediate values for the smallest u8 round trip.
class DiagU8Test : public UnitTest{
public:
    DiagU8Test()
        : UnitTest("Kernels::AudioPipeline - DIAG_U8(1ch x 2)")
    {}
    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        using Kernels::AudioStreamConverter::convert_from_Default;
        using Kernels::AudioStreamConverter::convert_to_Default;

        auto hexf = [](float f){
            uint32_t bits;
            std::memcpy(&bits, &f, 4);
            char buf[40];
            std::snprintf(buf, sizeof(buf), "%g(0x%08x)", (double)f, bits);
            return std::string(buf);
        };

        AlignedVector<uint8_t> ext0(2), ext1(2), ext2(2);
        AlignedVector<float> native(2), native2(2);
        ext0[0] = 95;
        ext0[1] = 203;

        convert_from_Default<uint8_t>(native.data(), ext0.data(), 2, 1.0f);
        convert_to_Default<uint8_t>(native.data(), ext1.data(), 2, 1.0f);
        Kernels::AudioStreamConverter::convert_from<uint8_t>(native2.data(), ext0.data(), 2, 1.0f);
        Kernels::AudioStreamConverter::convert_to<uint8_t>(native2.data(), ext2.data(), 2, 1.0f);

        //  Full pipeline, 2 samples.
        RawAudio2WorkingAudio raw2working(SampleFormat::UINT8, 44100, 1, 1.0f, 256);
        WorkingAudio2RawAudio working2raw(SampleFormat::UINT8, 44100, 1, 1.0f, 256);
        AlignedVector<uint8_t> pipeline_out(2);
        struct L : RawAudioStreamListener{
            AlignedVector<uint8_t>& data;
            L(SampleFormat t, size_t r, size_t c, AlignedVector<uint8_t>& p)
                : RawAudioStreamListener(t, r, c), data(p)
            {}
            virtual void push_samples(size_t frames, const void* p) override{
                const uint8_t* in = (const uint8_t*)p;
                for (size_t i = 0; i < frames; i++){
                    data[i] = in[i];
                }
            }
        } listener(SampleFormat::UINT8, 44100, 1, pipeline_out);
        raw2working.add_listener(working2raw);
        working2raw.add_listener(listener);
        raw2working.push_samples(2, ext0.data());

        return "DIAG from_default[0]=" + hexf(native[0]) +
            " to_default[0]=" + std::to_string(ext1[0]) +
            " from_dispatched[0]=" + hexf(native2[0]) +
            " to_dispatched[0]=" + std::to_string(ext2[0]) +
            " pipeline[0]=" + std::to_string(pipeline_out[0]) +
            " pipeline[1]=" + std::to_string(pipeline_out[1]);
    }
};

}

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

    database.add<DiagU8Test>();

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
