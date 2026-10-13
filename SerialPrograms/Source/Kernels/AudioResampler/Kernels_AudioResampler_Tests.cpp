/*  Audio Resampler Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cstring>
#include <random>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioResampler_Tests.h"
#include "Kernels_AudioResampler_BlockKernel_Default.h"
#ifdef PA_AutoDispatch_arm64_20_M1
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioResampler_BlockKernel_ARM64_NEON.h"
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



namespace{



class RecordingWorkingAudioListener : public AudioPipeline::WorkingAudioStreamListener{
public:
    RecordingWorkingAudioListener(size_t sample_rate, size_t channels, size_t capacity)
        : AudioPipeline::WorkingAudioStreamListener(sample_rate, channels)
        , m_capacity(capacity)
        , m_data(capacity * channels)
    {}

    virtual void push_samples(size_t samples, size_t stride, const float* data) override{
        for (size_t c = 0; c < channels(); c++){
            memcpy(
                m_data.data() + c * m_capacity + m_write,
                data + c * stride,
                samples * sizeof(float)
            );
        }
        m_write += samples;
    }

    const AlignedVector<float>& data() const{
        return m_data;
    }
    size_t written() const{
        return m_write;
    }
    size_t capacity_per_channel() const{
        return m_capacity;
    }


private:
    const size_t m_capacity;
    AlignedVector<float> m_data;
    size_t m_write = 0;
};



UnitTestResult run_resampler_stream(
    size_t in_rate, size_t out_rate, size_t channels,
    size_t samples, size_t push, uint32_t taps, double beta
){
    //  Generate random input.
    AlignedVector<float> input(samples * channels);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);
    for (size_t c = 0; c < input.size(); c++){
        input[c] = distribution(generator);
    }

    //  Small output buffer so a single push flushes multiple times.
    AudioResampler resampler(in_rate, out_rate, channels, 512, taps, beta);
    size_t capacity = samples * out_rate / in_rate + 4096;
    RecordingWorkingAudioListener output(out_rate, channels, capacity);
    resampler.add_listener(output);

    //  Push the input in chunks. The input is channel-planar: channel c is
    //  the block [c * samples, (c + 1) * samples).
    size_t offset = 0;
    while (offset < samples){
        size_t chunk = std::min(push, samples - offset);
        resampler.push_samples(chunk, samples, input.data() + offset);
        offset += chunk;
    }

    const size_t got = output.written();
    if (got == 0){
        return "No output produced.";
    }

    //  How many outputs should have been produced? The last windows can't
    //  complete if they don't fit in the pushed input, which costs up to the
    //  window span in input samples - scaled to the output rate.
    const double expected = (double)samples * out_rate / in_rate;
    const double tail = ((double)taps + 32.0) * out_rate / in_rate + 256.0;
    if ((double)got > expected + 2.0){
        return "Produced " + std::to_string(got) + " outputs, expected at most " +
            std::to_string((size_t)expected) + ".";
    }
    if ((double)got < expected - tail){
        return "Produced only " + std::to_string(got) + " outputs, expected at least " +
            std::to_string((size_t)(expected - tail)) + ".";
    }

    //  Reference: out_c[o] = sum_t weight[t][o] x in_c[round(o * in/out) + t].
    WeightTable reference(in_rate, out_rate, taps, beta, got + 64, taps);
    const uint32_t* ref_index = reference.in_sample_index();

    double max_diff = 0;
    size_t first_bad = 0;
    for (size_t o = 0; o < got; o++){
        const uint32_t in_base = ref_index[o];
        for (size_t c = 0; c < channels; c++){
            const float* in = input.data() + c * samples;
            float acc = 0;
            for (uint32_t t = 0; t < taps; t++){
                acc += reference.tap_start(t)[o] * in[in_base + t];
            }
            float diff = output.data()[c * capacity + o] - acc;
            if (diff < 0){
                diff = -diff;
            }
            if (diff > max_diff){
                max_diff = diff;
                first_bad = o * channels + c;
            }
        }
    }
    if (max_diff > 1e-5f){
        return "Max diff " + std::to_string(max_diff) + " at output " +
            std::to_string(first_bad) + ".";
    }

    return true;
}



#ifdef PA_AutoDispatch_arm64_20_M1



//
//  Runs run_block8_tap2_ARM64_NEON on a real weight table and random
//  input and compares every output sample to run_block8_Default.
//
class ResamplerBlockNEONTest : public UnitTest{
public:
    ResamplerBlockNEONTest(
        size_t in_sample_rate, size_t out_sample_rate,
        uint32_t taps, double beta
    )
        : UnitTest(
            "Kernels::AudioResampler - BlockNEON(" +
            std::to_string(in_sample_rate) + "->" + std::to_string(out_sample_rate) +
            ", " + std::to_string(taps) + " taps, beta " + std::to_string((int)beta) + ")"
        )
        , m_in_rate(in_sample_rate)
        , m_out_rate(out_sample_rate)
        , m_taps(taps)
        , m_beta(beta)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        size_t period = m_out_rate / std::gcd(m_in_rate, m_out_rate);
        size_t taps_padded = m_taps + (m_taps & 1);
        WeightTable table(m_in_rate, m_out_rate, m_taps, m_beta, period + 128, taps_padded);

        const uint32_t* index = table.in_sample_index();
        AlignedVector<float> input(index[period + 7] + taps_padded + 64);
        std::mt19937 generator(42);
        std::uniform_real_distribution<float> distribution(-1.f, 1.f);
        for (auto& x : input){
            x = distribution(generator);
        }

        //  Start mid-period so the block reads wrap past the period
        //  boundary into the table's extension.
        size_t out_index = period / 2;
        float out_neon[8];
        float out_ref[8];
        run_block8_tap2_ARM64_NEON(table, 0, input.data(), out_index, out_neon);
        run_block8_Default(table, 0, input.data(), out_index, out_ref);
        for (size_t c = 0; c < 8; c++){
            if (out_neon[c] != out_ref[c]){
                return "Mismatch at output " + std::to_string(c) + ": " +
                    std::to_string(out_neon[c]) + " != " + std::to_string(out_ref[c]) + ".";
            }
        }

        return true;
    }


private:
    const size_t m_in_rate;
    const size_t m_out_rate;
    const uint32_t m_taps;
    const double m_beta;
};




#endif



}



ResamplerStreamTest::ResamplerStreamTest(
    size_t in_sample_rate,
    size_t out_sample_rate,
    size_t channels,
    size_t samples,
    size_t push_size,
    uint32_t taps,
    double beta
)
    : UnitTest(
        "Kernels::AudioResampler - Stream(" +
        std::to_string(in_sample_rate) + "->" + std::to_string(out_sample_rate) +
        ", " + std::to_string(channels) + "ch, " + std::to_string(samples) +
        " samples, " + std::to_string(push_size) + " push, " +
        std::to_string(taps) + " taps, beta " + std::to_string((int)beta) + ")"
    )
    , m_in_rate(in_sample_rate)
    , m_out_rate(out_sample_rate)
    , m_channels(channels)
    , m_samples(samples)
    , m_push(push_size)
    , m_taps(taps)
    , m_beta(beta)
{}
UnitTestResult ResamplerStreamTest::run(Logger& logger, CancellableScope& scope) const{
    return run_resampler_stream(
        m_in_rate, m_out_rate, m_channels,
        m_samples, m_push, m_taps, m_beta
    );
}



void add_tests(UnitTestDatabase& database){
    struct Combo{
        size_t in_rate;
        size_t out_rate;
        size_t channels;
        size_t samples;
        size_t push;
        uint32_t taps;
        double beta;
    };
    //  1:1, upsample, downsample and irrational ratios, with chunk sizes
    //  that stress the carry buffer on either side of the block size.
    const Combo combos[] = {
        { 44100, 44100, 1, 4000, 100, 16, 9.0 },
        { 44100, 44100, 2, 4000, 250, 16, 9.0 },
        { 44100, 48000, 1, 4000, 100, 16, 9.0 },
        { 48000, 44100, 1, 4000, 250, 16, 9.0 },
        { 44100, 88200, 2, 4000, 4096, 16, 9.0 },
        { 88200, 44100, 2, 4000, 4096, 16, 9.0 },
        { 32000, 44100, 1, 4000, 7, 16, 5.0 },
        { 44100, 22050, 8, 4000, 250, 8, 9.0 },
        { 8000, 44100, 1, 4000, 100, 8, 9.0 },
    };
    for (auto& combo : combos){
        database.add<ResamplerStreamTest>(
            combo.in_rate, combo.out_rate, combo.channels,
            combo.samples, combo.push, combo.taps, combo.beta
        );
    }

#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        const struct{ size_t in_rate; size_t out_rate; uint32_t taps; double beta; } neons[] = {
            { 44100, 44100, 16, 9.0 },
            { 44100, 48000, 16, 9.0 },
            { 48000, 44100, 16, 9.0 },
            { 88200, 44100, 16, 9.0 },
            { 44100, 22050, 16, 9.0 },
            { 32000, 44100, 16, 5.0 },
            { 44100, 44100, 8, 9.0 },
            { 44100, 44100, 9, 9.0 },
            { 8000, 44100, 7, 9.0 },
        };
        for (auto& combo : neons){
            database.add<ResamplerBlockNEONTest>(combo.in_rate, combo.out_rate, combo.taps, combo.beta);
        }
    }
#endif
}



}
}
}
