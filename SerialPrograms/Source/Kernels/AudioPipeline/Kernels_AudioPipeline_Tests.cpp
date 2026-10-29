/*  Audio Pipeline Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include <vector>
#include <memory>
#include <cstring>
#include <QAudioFormat>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/AudioResampler/Kernels_AudioResampler.h"
#include "CommonFramework/AudioPipeline/Tools/ChannelMixer.h"
#include "Kernels_AudioPipeline.h"
#include "Kernels_RawAudio2WorkingAudio.h"
#include "Kernels_WorkingAudio2RawAudio.h"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_ChannelMixer_TestInfra.h"
#include "Kernels_ChannelMixer.h"
#include "Kernels_WorkingAudio2WorkingAudio.h"

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



//
//  WorkingAudio2WorkingAudio vs. the hand-wired chain of the same kernels.
//
//  The all-in-one node orders its internals by channel count:
//    - in_channels >= out_channels: mixer (at the input rate) -> resampler,
//    - in_channels <  out_channels: resampler (at the input channel count) ->
//      mixer (at the output rate).
//  Identical input is fed through the node and through that exact hand-wired
//  chain, and the outputs are asserted bit-identical. The two orderings are
//  mathematically equivalent (both are LTI - mixing and FIR resampling
//  commute), so this pins the node's wiring, the shared matrix construction
//  and the stride forwarding.
//
class Working2WorkingOrderTest : public UnitTest{
public:
    Working2WorkingOrderTest(size_t in_rate, size_t out_rate, size_t in_ch, size_t out_ch)
        : UnitTest(
            "Kernels::AudioPipeline - Working2WorkingOrder(" +
            std::to_string(in_rate) + "->" + std::to_string(out_rate) + "Hz, " +
            std::to_string(in_ch) + "ch->" + std::to_string(out_ch) + "ch)"
        )
        , m_in_rate(in_rate)
        , m_out_rate(out_rate)
        , m_in_ch(in_ch)
        , m_out_ch(out_ch)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t frames = 8192;

        //  Planar input: channel c lives at [c * frames .. c * frames + frames).
        AlignedVector<float> input(m_in_ch * frames);
        std::mt19937 generator(42);
        std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
        for (size_t c = 0; c < m_in_ch; c++){
            for (size_t i = 0; i < frames; i++){
                input[c * frames + i] = distribution(generator);
            }
        }

        auto channel_format = [](size_t channels){
            QAudioFormat format;
            format.setChannelCount((int)channels);
            format.setChannelConfig(
                channels == 1
                    ? QAudioFormat::ChannelConfig::ChannelConfigMono
                    : QAudioFormat::ChannelConfig::ChannelConfigStereo
            );
            return format;
        };
        const QAudioFormat in_format = channel_format(m_in_ch);
        const QAudioFormat out_format = channel_format(m_out_ch);

        //  The node under test.
        std::vector<float> node_matrix = build_channel_mixer_matrix(in_format, out_format);
        if (node_matrix.size() != m_in_ch * m_out_ch){
            return "No channel mixer matrix for this conversion.";
        }
        WorkingAudio2WorkingAudio node(
            logger, m_in_rate, m_out_rate, m_in_ch, m_out_ch, node_matrix
        );
        RecordingWorkingAudioListener node_output(m_out_rate, m_out_ch, frames * 2);
        node.add_listener(node_output);

        //  The hand-wired chain, in the node's internal ordering.
        std::vector<float> chain_matrix = build_channel_mixer_matrix(in_format, out_format);
        std::unique_ptr<ChannelMixer> chain_mixer;
        std::unique_ptr<AudioResampler::AudioResampler> chain_resampler;
        WorkingAudioNode* chain_head = nullptr;
        WorkingAudioNode* chain_tail = nullptr;
        if (m_in_ch >= m_out_ch){
            chain_mixer.reset(new ChannelMixer(
                logger, m_in_rate, m_in_ch, m_out_ch, chain_matrix
            ));
            chain_head = chain_mixer.get();
            if (m_in_rate != m_out_rate){
                chain_resampler.reset(new AudioResampler::AudioResampler(
                    logger, m_in_rate, m_out_rate, m_out_ch
                ));
                chain_mixer->add_listener(*chain_resampler);
                chain_tail = chain_resampler.get();
            }else{
                chain_tail = chain_mixer.get();
            }
        }else{
            chain_mixer.reset(new ChannelMixer(
                logger, m_out_rate, m_in_ch, m_out_ch, chain_matrix
            ));
            chain_tail = chain_mixer.get();
            if (m_in_rate != m_out_rate){
                chain_resampler.reset(new AudioResampler::AudioResampler(
                    logger, m_in_rate, m_out_rate, m_in_ch
                ));
                chain_resampler->add_listener(*chain_tail);
                chain_head = chain_resampler.get();
            }else{
                chain_head = chain_mixer.get();
            }
        }
        RecordingWorkingAudioListener chain_output(m_out_rate, m_out_ch, frames * 2);
        chain_tail->add_listener(chain_output);

        //  Feed both in two chunks, each with a padded stride (stride >
        //  samples exercises the channel-block layout handling).
        size_t offset = 0;
        const size_t chunk_sizes[] = {5000, frames - 5000};
        for (size_t chunk : chunk_sizes){
            AlignedVector<float> padded(m_in_ch * (chunk + 3));
            for (size_t c = 0; c < m_in_ch; c++){
                for (size_t i = 0; i < chunk; i++){
                    padded[c * (chunk + 3) + i] = input[c * frames + offset + i];
                }
            }
            node.push_samples(chunk, chunk + 3, padded.data());
            chain_head->push_samples(chunk, chunk + 3, padded.data());
            offset += chunk;
        }

        const size_t written = node_output.frames_written();
        if (written == 0){
            return "No output was produced.";
        }
        if (written != chain_output.frames_written()){
            return "Frame count mismatch: node wrote " + std::to_string(written) +
                " frames, chain wrote " + std::to_string(chain_output.frames_written()) + ".";
        }
        const auto& a = node_output.data();
        const auto& b = chain_output.data();
        const size_t count = written * m_out_ch;
        if (std::memcmp(a.data(), b.data(), count * sizeof(float)) != 0){
            for (size_t i = 0; i < count; i++){
                if (a[i] != b[i]){
                    return "Mismatch at " + std::to_string(i) + ": " + tostr_default(a[i]) +
                        " != " + tostr_default(b[i]) + ".";
                }
            }
            return "Bit-level mismatch with equal values (sign of zero or NaN payload).";
        }

        return true;
    }


private:
    const size_t m_in_rate;
    const size_t m_out_rate;
    const size_t m_in_ch;
    const size_t m_out_ch;
};



//
//  ChannelMixer with pushes larger than its internal buffer.
//
//  mix_channels() strides the output buffer at the buffer size, so
//  push_samples() must chunk larger pushes internally. An identity mix
//  must pass every sample through unchanged, regardless of how the
//  push is split up.
//
class MixerOverBufferPushTest : public UnitTest{
public:
    MixerOverBufferPushTest(size_t channels, size_t samples)
        : UnitTest(
            "Kernels::AudioPipeline - MixerOverBufferPush(" +
            std::to_string(channels) + "ch x " + std::to_string(samples) + ")"
        )
        , m_channels(channels)
        , m_samples(samples)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        std::vector<float> matrix(m_channels * m_channels, 0.0f);
        for (size_t c = 0; c < m_channels; c++){
            matrix[c + m_channels * c] = 1.0f;
        }

        GuardPageBuffer<float> input = random_float_input(m_samples * m_channels, 42);

        const size_t sample_rate = 44100;
        ChannelMixer mixer(logger, sample_rate, m_channels, m_channels, std::move(matrix));
        RecordingWorkingAudioListener listener(sample_rate, m_channels, m_samples);
        mixer.add_listener(listener);

        //  A single push larger than the mixer's internal buffer (4096 + 256).
        mixer.push_samples(m_samples, m_samples, input.data());

        if (listener.frames_written() != m_samples){
            return "Received " + std::to_string(listener.frames_written()) + " frames, expected " +
                std::to_string(m_samples) + ".";
        }

        for (size_t i = 0; i < m_samples; i++){
            for (size_t c = 0; c < m_channels; c++){
                float expected = input[c * m_samples + i];
                if (expected != listener.data()[i * m_channels + c]){
                    return "Mismatch at (" + std::to_string(i) + ", " + std::to_string(c) + "): " +
                        tostr_default(expected) + " != " +
                        tostr_default(listener.data()[i * m_channels + c]) + ".";
                }
            }
        }

        return true;
    }


private:
    const size_t m_channels;
    const size_t m_samples;
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

    //  Pushes larger than the mixer's internal buffer get chunked.
    for (size_t channels = 1; channels <= 3; channels++){
        database.add<MixerOverBufferPushTest>(channels, 10000);
    }

    //  WorkingAudio2WorkingAudio vs. the hand-wired chain: both internal
    //  orderings (mix -> resample, resample -> mix), plus the mix-only,
    //  identity-matrix + resample and pure pass-through (identity matrix,
    //  same rate) cases.
    database.add<Working2WorkingOrderTest>(44100, 48000, 2, 1);
    database.add<Working2WorkingOrderTest>(44100, 48000, 1, 2);
    database.add<Working2WorkingOrderTest>(48000, 48000, 2, 1);
    database.add<Working2WorkingOrderTest>(48000, 48000, 1, 2);
    database.add<Working2WorkingOrderTest>(48000, 44100, 2, 2);
    database.add<Working2WorkingOrderTest>(48000, 48000, 2, 2);
}



}
}
}
