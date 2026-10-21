/*  Audio Pipeline Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioPipeline.h"
#include "Kernels_RawAudio2WorkingAudio.h"
#include "Kernels_WorkingAudio2RawAudio.h"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_ChannelMixer_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



namespace{



class RecordingWorkingAudioListener : public WorkingAudioStreamListener{
public:
    RecordingWorkingAudioListener(size_t sample_rate, size_t channels, size_t total_frames)
        : WorkingAudioStreamListener(sample_rate, channels)
        , m_data(total_frames * channels)
    {}

    virtual void push_samples(size_t samples, size_t stride, const float* data) override{
        for (size_t i = 0; i < samples; i++){
            for (size_t c = 0; c < channels(); c++){
                m_data[m_write_index + i * channels() + c] = data[c * stride + i];
            }
        }
        m_write_index += samples * channels();
    }

    const AlignedVector<float>& data() const{
        return m_data;
    }
    size_t frames_written() const{
        return m_write_index / channels();
    }


private:
    AlignedVector<float> m_data;
    size_t m_write_index = 0;
};



class RecordingRawFloatListener : public RawAudioStreamListener{
public:
    RecordingRawFloatListener(size_t sample_rate, size_t channels, size_t total_frames)
        : RawAudioStreamListener(SampleFormat::FLOAT32, sample_rate, channels)
        , m_data(total_frames * channels)
    {}

    virtual void push_samples(size_t frames, const void* data) override{
        const float* input = (const float*)data;
        for (size_t c = 0; c < frames * channels(); c++){
            m_data[m_write_index + c] = input[c];
        }
        m_write_index += frames * channels();
    }

    const AlignedVector<float>& data() const{
        return m_data;
    }
    size_t frames_written() const{
        return m_write_index / channels();
    }


private:
    AlignedVector<float> m_data;
    size_t m_write_index = 0;
};



//  The buffer ends right before a guard page, so a read past the end faults.
GuardPageBuffer<float> random_float_input(size_t total_samples, uint32_t seed){
    GuardPageBuffer<float> input(total_samples);
    std::mt19937 generator(seed);
    //  Integer values, all exactly representable in float.
    std::uniform_int_distribution<long long> distribution(-128, 128);
    for (size_t c = 0; c < total_samples; c++){
        input[c] = (float)distribution(generator);
    }
    return input;
}



}



//
//  Pushes random float samples through a RawAudio2WorkingAudio and checks
//  that the gain actually reaches the output (the FLOAT32 zero-copy fast
//  path must not drop the multiplier).
//
class RawFloatGainTest : public UnitTest{
public:
    RawFloatGainTest(size_t channels, size_t samples, float multiplier)
        : UnitTest(
            "Kernels::AudioPipeline - RawFloatGain(" + std::to_string(channels) + "ch x " +
            std::to_string(samples) + ", m=" + tostr_default(multiplier) + ")"
        )
        , m_channels(channels)
        , m_samples(samples)
        , m_multiplier(multiplier)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t sample_rate = 44100;
        const size_t buffer_size = 256; //  Small buffer so the sweep hits multiple chunks.

        GuardPageBuffer<float> input = random_float_input(m_samples * m_channels, 42);
        RawAudio2WorkingAudio raw2working(
            SampleFormat::FLOAT32, sample_rate, m_channels, m_multiplier, buffer_size
        );
        RecordingWorkingAudioListener output(sample_rate, m_channels, m_samples);
        raw2working.add_listener(output);

        raw2working.push_samples(m_samples, input.data());

        if (output.frames_written() != m_samples){
            return "Received " + std::to_string(output.frames_written()) + " frames, expected " +
                std::to_string(m_samples) + ".";
        }

        for (size_t c = 0; c < input.size(); c++){
            float expected = input[c] * m_multiplier;
            if (output.data()[c] != expected){
                return "Mismatch at " + std::to_string(c) + ": " +
                    tostr_default(input[c]) + " * " + tostr_default(m_multiplier) + " = " +
                    tostr_default(expected) + " != " + tostr_default(output.data()[c]) + ".";
            }
        }

        return true;
    }


private:
    const size_t m_channels;
    const size_t m_samples;
    const float m_multiplier;
};



//
//  Pushes random float samples through a WorkingAudio2RawAudio and checks
//  that the gain actually reaches the output (the FLOAT32 zero-copy fast
//  path must not drop the multiplier).
//
class WorkingFloatGainTest : public UnitTest{
public:
    WorkingFloatGainTest(size_t channels, size_t samples, float multiplier)
        : UnitTest(
            "Kernels::AudioPipeline - WorkingFloatGain(" + std::to_string(channels) + "ch x " +
            std::to_string(samples) + ", m=" + tostr_default(multiplier) + ")"
        )
        , m_channels(channels)
        , m_samples(samples)
        , m_multiplier(multiplier)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t sample_rate = 44100;
        const size_t buffer_size = 256; //  Small buffer so the sweep hits multiple chunks.

        //  Working format: channel-strided, one block per channel.
        GuardPageBuffer<float> input = random_float_input(m_samples * m_channels, 42);
        WorkingAudio2RawAudio working2raw(
            SampleFormat::FLOAT32, sample_rate, m_channels, m_multiplier, buffer_size
        );
        RecordingRawFloatListener output(sample_rate, m_channels, m_samples);
        working2raw.add_listener(output);

        working2raw.push_samples(m_samples, m_samples, input.data());

        if (output.frames_written() != m_samples){
            return "Received " + std::to_string(output.frames_written()) + " frames, expected " +
                std::to_string(m_samples) + ".";
        }

        for (size_t i = 0; i < m_samples; i++){
            for (size_t c = 0; c < m_channels; c++){
                float expected = input[c * m_samples + i] * m_multiplier;
                if (output.data()[i * m_channels + c] != expected){
                    return "Mismatch at " + std::to_string(i * m_channels + c) + ": " +
                        tostr_default(input[c * m_samples + i]) + " * " + tostr_default(m_multiplier) +
                        " = " + tostr_default(expected) + " != " +
                        tostr_default(output.data()[i * m_channels + c]) + ".";
                }
            }
        }

        return true;
    }


private:
    const size_t m_channels;
    const size_t m_samples;
    const float m_multiplier;
};



//
//  Checks that set_multiplier() applies live: each batch is recorded with
//  the gain that was in effect when it was pushed, for both nodes.
//
class FloatGainLiveUpdateTest : public UnitTest{
public:
    FloatGainLiveUpdateTest()
        : UnitTest("Kernels::AudioPipeline - FloatGainLiveUpdate(3 batches)")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t sample_rate = 44100;
        const size_t channels = 1;
        const size_t batch = 1000;
        const size_t buffer_size = 256;

        std::vector<float> gains = {1.0f, 0.5f, 1.0f};
        std::vector<GuardPageBuffer<float>> inputs;
        for (size_t b = 0; b < gains.size(); b++){
            inputs.emplace_back(random_float_input(batch, 42));
        }

        //  RawAudio2WorkingAudio.
        {
            RawAudio2WorkingAudio raw2working(
                SampleFormat::FLOAT32, sample_rate, channels, gains[0], buffer_size
            );
            RecordingWorkingAudioListener output(sample_rate, channels, batch * 3);
            raw2working.add_listener(output);

            for (size_t b = 0; b < inputs.size(); b++){
                raw2working.set_multiplier(gains[b]);
                raw2working.push_samples(batch, inputs[b].data());
            }

            if (output.frames_written() != batch * 3){
                return "RawAudio2WorkingAudio: received " + std::to_string(output.frames_written()) +
                    " frames, expected " + std::to_string(batch * 3) + ".";
            }
            for (size_t b = 0; b < inputs.size(); b++){
                for (size_t i = 0; i < batch; i++){
                    float expected = inputs[b][i] * gains[b];
                    size_t index = b * batch + i;
                    if (output.data()[index] != expected){
                        return "RawAudio2WorkingAudio batch " + std::to_string(b) +
                            " mismatch at " + std::to_string(i) + ": " + tostr_default(expected) +
                            " != " + tostr_default(output.data()[index]) + ".";
                    }
                }
            }
        }

        //  WorkingAudio2RawAudio.
        {
            WorkingAudio2RawAudio working2raw(
                SampleFormat::FLOAT32, sample_rate, channels, gains[0], buffer_size
            );
            RecordingRawFloatListener output(sample_rate, channels, batch * 3);
            working2raw.add_listener(output);

            for (size_t b = 0; b < inputs.size(); b++){
                working2raw.set_multiplier(gains[b]);
                working2raw.push_samples(batch, batch, inputs[b].data());
            }

            if (output.frames_written() != batch * 3){
                return "WorkingAudio2RawAudio: received " + std::to_string(output.frames_written()) +
                    " frames, expected " + std::to_string(batch * 3) + ".";
            }
            for (size_t b = 0; b < inputs.size(); b++){
                for (size_t i = 0; i < batch; i++){
                    float expected = inputs[b][i] * gains[b];
                    size_t index = b * batch + i;
                    if (output.data()[index] != expected){
                        return "WorkingAudio2RawAudio batch " + std::to_string(b) +
                            " mismatch at " + std::to_string(i) + ": " + tostr_default(expected) +
                            " != " + tostr_default(output.data()[index]) + ".";
                    }
                }
            }
        }

        return true;
    }
};



void add_tests(UnitTestDatabase& database){
    //  Raw -> Working -> Raw round-trip.
    for (size_t channels = 1; channels <= 4; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            for (float multiplier = 0.1f; multiplier <= 1.0f; multiplier += 0.1f){
                database.add<RoundTripTest>(SampleFormat::UINT8, channels, samples, multiplier);
                database.add<RoundTripTest>(SampleFormat::SINT16, channels, samples, multiplier);
                database.add<RoundTripTest>(SampleFormat::SINT32, channels, samples, multiplier);
                database.add<RoundTripTest>(SampleFormat::FLOAT32, channels, samples, multiplier);
            }
        }
    }

    //  FLOAT32 gain: the zero-copy fast paths must not drop the multiplier.
    //  (m == 1.0 exercises the fast path; the others the conversion path.)
    for (size_t channels = 1; channels <= 2; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            const float gain_multipliers[] = {0.1f, 0.4f, 0.7f, 1.0f};
            for (float multiplier : gain_multipliers){
                database.add<RawFloatGainTest>(channels, samples, multiplier);
                database.add<WorkingFloatGainTest>(channels, samples, multiplier);
            }
        }
    }

    //  set_multiplier() applies live, switching the fast path on and off.
    database.add<FloatGainLiveUpdateTest>();

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
