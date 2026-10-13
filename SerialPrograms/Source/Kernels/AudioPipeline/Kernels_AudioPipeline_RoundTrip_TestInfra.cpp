/*  Audio Pipeline - Raw<->Working Round-Trip Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <limits>
#include <random>
#include <type_traits>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_RawAudio2WorkingAudio.h"
#include "Kernels_WorkingAudio2RawAudio.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




namespace{



template <typename Type>
class RecordingRawAudioListener : public RawAudioStreamListener{
public:
    RecordingRawAudioListener(SampleFormat sample_type, size_t sample_rate, size_t channels, size_t total_samples)
        : RawAudioStreamListener(sample_type, sample_rate, channels)
        , m_data(total_samples)
    {}

    virtual void push_samples(size_t frames, const void* data) override{
        const Type* input = (const Type*)data;
        size_t count = frames * channels();
        for (size_t c = 0; c < count; c++){
            m_data[m_write_index + c] = input[c];
        }
        m_write_index += count;
    }

    const AlignedVector<Type>& data() const{
        return m_data;
    }
    size_t written() const{
        return m_write_index;
    }


private:
    AlignedVector<Type> m_data;
    size_t m_write_index = 0;
};



std::string sample_typestr(SampleFormat sample_type){
    switch (sample_type){
    case SampleFormat::UINT8:   return "u8";
    case SampleFormat::SINT16:  return "s16";
    case SampleFormat::SINT32:  return "s32";
    case SampleFormat::FLOAT32: return "f32";
    default:;
    }
    return "?";
}



template <typename Type>
UnitTestResult run_round_trip(SampleFormat sample_type, size_t channels, size_t samples){
    //  Generate random input.
    AlignedVector<Type> input(samples * channels);
    std::mt19937 generator(42);
    if constexpr (std::is_same_v<Type, float>){
        //  Random integer values, all exactly representable.
        std::uniform_int_distribution<long long> distribution(-128, 128);
        for (size_t c = 0; c < input.size(); c++){
            input[c] = (float)distribution(generator);
        }
    }else{
        std::uniform_int_distribution<long long> distribution(
            std::numeric_limits<Type>::min(),
            std::numeric_limits<Type>::max()
        );
        for (size_t c = 0; c < input.size(); c++){
            input[c] = (Type)distribution(generator);
            if constexpr (std::is_same_v<Type, int32_t>){
                //  Truncate some precision so the round-trip is lossless.
                input[c] &= 0xfffff000;
            }
        }
    }

    const size_t sample_rate = 44100;

    //  Small buffer so the sample-size sweep hits multiple chunks.
    const size_t buffer_size = 256;
    RawAudio2WorkingAudio raw2working(sample_type, sample_rate, channels, buffer_size);
    WorkingAudio2RawAudio working2raw(sample_type, sample_rate, channels, 1.0f, buffer_size);
    RecordingRawAudioListener<Type> output(sample_type, sample_rate, channels, samples * channels);
    raw2working.add_listener(working2raw);
    working2raw.add_listener(output);

    raw2working.push_samples(samples, input.data());

    if (output.written() != samples * channels){
        return "Received " + std::to_string(output.written()) + " samples, expected " +
            std::to_string(samples * channels) + ".";
    }

    //  Check for errors.
    for (size_t c = 0; c < input.size(); c++){
        if (input[c] != output.data()[c]){
            return "Mismatch at " + std::to_string(c) + ": " +
                std::to_string(input[c]) + " != " + std::to_string(output.data()[c]);
        }
    }

    return true;
}



}




RoundTripTest::RoundTripTest(
    SampleFormat sample_type,
    size_t channels,
    size_t samples
)
    : UnitTest(
        "Kernels::AudioPipeline - RoundTrip_" + sample_typestr(sample_type) +
        "(" + std::to_string(channels) + "ch x " + std::to_string(samples) + ")"
    )
    , m_sample_type(sample_type)
    , m_channels(channels)
    , m_samples(samples)
{}
UnitTestResult RoundTripTest::run(Logger& logger, CancellableScope& scope) const{
    switch (m_sample_type){
    case SampleFormat::UINT8:
        return run_round_trip<uint8_t>(m_sample_type, m_channels, m_samples);
    case SampleFormat::SINT16:
        return run_round_trip<int16_t>(m_sample_type, m_channels, m_samples);
    case SampleFormat::SINT32:
        return run_round_trip<int32_t>(m_sample_type, m_channels, m_samples);
    case SampleFormat::FLOAT32:
        return run_round_trip<float>(m_sample_type, m_channels, m_samples);
    default:
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Unknown sample type.");
    }
}




}
}
}
