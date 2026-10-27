/*  Audio Resampler Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <numbers>
#include <random>
#include <utility>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/KaiserWindow/Kernels_KaiserWindow.h"
#include "Kernels_AudioResampler_BlockKernels.h"
#include "Kernels_AudioResampler_Tests.h"
#include "Kernels_AudioResampler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



namespace{



//
//  Scalar reference for one output sample:
//      out[out_index + c] = sum_t weight[t][out_index + c] x
//          input[index[out_index + c] + t]
//
float block_reference(
    const WeightTable& table, const float* input, size_t out_index, size_t c
){
    const uint32_t* index = table.in_sample_index();
    const uint32_t base = index[out_index + c];
    float acc = 0;
    for (size_t t = 0; t < table.taps(); t++){
        acc += table.tap_start(t)[out_index + c] * input[base + t];
    }
    return acc;
}
AlignedVector<float> reference_lanes(
    const WeightTable& table, const float* input, size_t out_index, size_t block
){
    AlignedVector<float> out(block);
    for (size_t c = 0; c < block; c++){
        out[c] = block_reference(table, input, out_index, c);
    }
    return out;
}
std::string format_lanes(const float* v, size_t n){
    std::string s;
    const size_t count = n < 4 ? n : 4;
    for (size_t i = 0; i < count; i++){
        if (i > 0){
            s += ", ";
        }
        s += std::to_string(v[i]);
    }
    return s;
}


//
//  Number of ULPs between two floats, in a monotonic bit ordering.
//  Returns a distance larger than any real one if either value is NaN.
//
int64_t ulp_distance(float a, float b){
    if (a == b){
        return 0;
    }
    if (a != a || b != b){
        return INT64_MAX;
    }
    const int32_t ia = std::bit_cast<int32_t>(a);
    const int32_t ib = std::bit_cast<int32_t>(b);
    const int64_t ma = ia < 0 ? -(int64_t)(uint32_t)ia : ia;
    const int64_t mb = ib < 0 ? -(int64_t)(uint32_t)ib : ib;
    return ma > mb ? ma - mb : mb - ma;
}
//
//  The SIMD kernels use the same coefficients as the reference; only the
//  floating-point rounding can differ (FMA contraction, accumulation
//  order). The tolerance is 32 ULPs of the result magnitude, anchored at
//  1.0 so near-zero results keep an absolute floor. Any indexing or
//  coefficient bug shows up as an O(0.01) difference, far outside this.
//
bool within_ulp_tolerance(float got, float want, int64_t& ulp_diff){
    ulp_diff = ulp_distance(got, want);
    if (std::isinf(got) || std::isinf(want)){
        return got == want;
    }
    const float scale = std::max(std::max(std::fabs(got), std::fabs(want)), 1.0f);
    const float ulp = std::nextafterf(scale, INFINITY) - scale;
    return std::fabs(got - want) <= 32.0f * ulp;
}


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
    Logger& logger,
    size_t in_rate, size_t out_rate, size_t channels,
    size_t samples, size_t push, uint32_t taps, double beta
){
    //  Generate random input. It ends right before a guard page, so a read
    //  past the end faults.
    GuardPageBuffer<float> input(samples * channels);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);
    for (size_t c = 0; c < input.size(); c++){
        input[c] = distribution(generator);
    }

    //  Small output buffer so a single push flushes multiple times.
    AudioResampler resampler(logger, in_rate, out_rate, channels, 512, taps, beta);
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

    //  Reference: out_c[o] = sum_t weight[t][o] x in_c[round(o * in/out) + t].
    //  The reference only reads index/weight columns 0..got - 1, so the
    //  block size is got itself and the tap group 1.
    WeightTable reference(in_rate, out_rate, taps, beta, got, 1);
    const uint32_t* ref_index = reference.in_sample_index();

    //  How many outputs should have been produced? The resampler may be
    //  holding back the unprocessed input tail: at most the next block's
    //  window span plus its taps plus the kernel's read overshoot
    //  (block - 1) input samples, scaled to the output rate. The span
    //  grows with the dispatched block size (up to 128 samples on the
    //  wide permute kernels), so bound it exactly from the table.
    const size_t block = resampler.block_size();
    size_t max_span = 0;
    for (size_t wr = 0; wr < reference.period_out(); wr++){
        max_span = std::max(max_span, (size_t)(ref_index[wr + block - 1] - ref_index[wr]));
    }
    const double expected = (double)samples * out_rate / in_rate;
    const double tail = ((double)max_span + taps + block) * out_rate / in_rate + 32.0;
    if ((double)got > expected + 2.0){
        return "Produced " + std::to_string(got) + " outputs, expected at most " +
            std::to_string((size_t)expected) + ".";
    }
    if ((double)got < expected - tail){
        return "Produced only " + std::to_string(got) + " outputs, expected at least " +
            std::to_string((size_t)(expected - tail)) + ".";
    }

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



}



//  Per-ISA test registration, defined in the matching block kernel
//  translation unit next to the kernel it tests.
void add_tests_x64_SSE2(UnitTestDatabase& database);
void add_tests_x64_FMA3(UnitTestDatabase& database);
void add_tests_x64_AVX512(UnitTestDatabase& database);
void add_tests_ARM64_NEON(UnitTestDatabase& database);




BlockKernelTest::BlockKernelTest(
    const char* p_kernel,
    KernelFn p_kernel_fn,
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps,
    size_t sample_block,
    size_t tap_block,
    size_t out_index
)
    : UnitTest(
        "Kernels::AudioResampler - Block(" + std::string(p_kernel) +
        ", " + std::to_string(in_sample_rate) + "->" + std::to_string(out_sample_rate) +
        ", " + std::to_string(taps) + " taps) @" + std::to_string(out_index)
      )
    , m_kernel_fn(p_kernel_fn)
    , m_in_rate(in_sample_rate)
    , m_out_rate(out_sample_rate)
    , m_taps(taps)
    , m_block(sample_block)
    , m_tap_block(tap_block)
    , m_out_index(out_index)
{}
UnitTestResult BlockKernelTest::run(Logger& logger, CancellableScope& scope) const{
    (void)logger;
    (void)scope;

    WeightTable table(
        m_in_rate, m_out_rate, m_taps, 9.0,
        m_block, m_tap_block
    );
    if (m_out_index >= table.period_out()){
        return "out_index must wrap to the period.";
    }

    //  Deterministic pseudo-random input. input[i] is absolute stream
    //  sample i, so the kernel's in_index is 0 (k = 0 periods in).
    //  The index columns keep counting past the period, so a block that
    //  wraps the period reads samples up to about period_in + block + taps.
    //  The input and the output both end right before a guard page.
    GuardPageBuffer<float> input(table.period_in() + m_block + m_taps + 128);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    for (size_t i = 0; i < input.size(); i++){
        input[i] = distribution(generator);
    }

    GuardPageBuffer<float> got(m_block);
    m_kernel_fn(table, 0, input.data(), m_out_index, got.data());

    AlignedVector<float> want = reference_lanes(table, input.data(), m_out_index, m_block);

    for (size_t c = 0; c < m_block; c++){
        int64_t ulp = 0;
        if (!within_ulp_tolerance(got[c], want[c], ulp)){
            return "Kernels disagree; max diff " + std::to_string(std::fabs(got[c] - want[c])) +
                " at output " + std::to_string(c) +
                "; got[" + std::to_string(c) + "] = " + std::to_string(got[c]) +
                ", want[" + std::to_string(c) + "] = " + std::to_string(want[c]) +
                " (" + std::to_string(ulp) + " ULP); got0..3 = [" + format_lanes(got.data(), m_block) +
                "], want0..3 = [" + format_lanes(want.data(), m_block) + "]";
        }
    }

    return true;
}



BlockDispatchTest::BlockDispatchTest(
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps
)
    : UnitTest(
        "Kernels::AudioResampler - Dispatch(" + std::to_string(in_sample_rate) +
        "->" + std::to_string(out_sample_rate) + ", " +
        std::to_string(taps) + " taps)"
      )
    , m_in_rate(in_sample_rate)
    , m_out_rate(out_sample_rate)
    , m_taps(taps)
{}
UnitTestResult BlockDispatchTest::run(Logger& logger, CancellableScope& scope) const{
    (void)logger;
    (void)scope;

    BlockKernel bk = get_block_kernel(m_in_rate, m_out_rate);
    if (bk.fp == nullptr){
        return "get_block_kernel returned a null function pointer.";
    }

    //  Table sized exactly to the kernel's block.
    WeightTable table(
        m_in_rate, m_out_rate, m_taps, 9.0,
        bk.sample_block, bk.tap_block
    );

    //  The kernel's input reads may extend sample_block - 1 samples past
    //  the last window's start. The input and the output both end right
    //  before a guard page.
    const uint32_t* index = table.in_sample_index();
    GuardPageBuffer<float> input(index[bk.sample_block - 1] + m_taps + bk.sample_block + 64);
    std::mt19937 generator(42);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    for (size_t i = 0; i < input.size(); i++){
        input[i] = distribution(generator);
    }

    GuardPageBuffer<float> got(bk.sample_block);
    bk.fp(table, 0, input.data(), 0, got.data());

    AlignedVector<float> want = reference_lanes(table, input.data(), 0, bk.sample_block);

    for (size_t c = 0; c < bk.sample_block; c++){
        int64_t ulp = 0;
        if (!within_ulp_tolerance(got[c], want[c], ulp)){
            return "Dispatched kernel disagrees; max diff " +
                std::to_string(std::fabs(got[c] - want[c])) +
                " at output " + std::to_string(c) +
                "; got[" + std::to_string(c) + "] = " + std::to_string(got[c]) +
                ", want[" + std::to_string(c) + "] = " + std::to_string(want[c]) +
                " (" + std::to_string(ulp) + " ULP)";
        }
    }

    return true;
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
        logger,
        m_in_rate, m_out_rate, m_channels,
        m_samples, m_push, m_taps, m_beta
    );
}



WeightTableTimeShiftTest::WeightTableTimeShiftTest(
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps
)
    : UnitTest(
        "Kernels::AudioResampler - TimeShift(" + std::to_string(in_sample_rate) +
        "->" + std::to_string(out_sample_rate) + ", " +
        std::to_string(taps) + " taps)"
    )
    , m_in_rate(in_sample_rate)
    , m_out_rate(out_sample_rate)
    , m_taps(taps)
{}
UnitTestResult WeightTableTimeShiftTest::run(Logger& logger, CancellableScope& scope) const{
    (void)logger;
    (void)scope;

    WeightTable table(m_in_rate, m_out_rate, m_taps, 9.0, 8, 1);

    //  The shift must be the 1st-tap to median-tap distance, in seconds
    //  of the input stream.
    const double want_shift = ((double)m_taps - 1.0) * 0.5 / (double)m_in_rate;
    if (table.time_shift() != want_shift){
        return "time_shift() = " + std::to_string(table.time_shift()) +
            ", expected " + std::to_string(want_shift) + " seconds."
        ;
    }

    //  The table must actually reconstruct at the shifted time: a delta
    //  at input sample p0 produces, at output sample c, the windowed sinc
    //  centered on input time p0 evaluated at the output's time,
    //  c * ratio + (taps - 1) / 2 input samples (zero when p0 is outside
    //  c's input window). The delta makes exactly one tap nonzero, so the
    //  output is that tap's stored float weight, no accumulation error.
    using namespace Kernels::KaiserWindow;
    const double ratio = (double)m_in_rate / m_out_rate;
    const double shift = ((double)m_taps - 1.0) * 0.5;
    const double tap_radius = m_taps * 0.5;
    const double scale = (m_in_rate > m_out_rate)
        ? ((double)m_out_rate / m_in_rate)
        : 1.0;
    const double ibeta = bessel_i0(9.0);
    auto weight_at = [&](double x)->double{
        //  The table constructor's windowed sinc, in doubles.
        if (x == 0.0){
            return scale;
        }
        if (std::fabs(x) >= tap_radius){
            return 0.0;
        }
        double arg = std::numbers::pi * x * scale;
        double sinc = std::sin(arg) / arg;
        double kaiser = x / tap_radius;
        kaiser *= kaiser;
        kaiser = 1 - kaiser;
        kaiser = kaiser <= 0 ? 0 : std::sqrt(kaiser);
        kaiser *= 9.0;
        kaiser = bessel_i0(kaiser) / ibeta;
        double w = kaiser * sinc * scale;
        if (std::abs(w) < 1e-12){
            w = 0;
        }
        return w;
    };

    //  Well inside the stream, away from the period's start edge.
    const size_t p0 = table.period_in() * 4 + m_taps + 128;
    AlignedVector<float> input(p0 + 1024);
    memset(input.data(), 0, input.size() * sizeof(float));
    input[p0] = 1.0f;

    //  Output sample c from the table alone, with the resampler's
    //  wrapped-period cursor arithmetic.
    auto out_sample = [&](size_t c)->std::pair<float, size_t>{
        const size_t poe = table.period_out_extended();
        const size_t wr = c % poe;
        const size_t periods = c / poe;
        const size_t base = table.in_sample_index()[wr]
            + periods * poe * table.period_in() / table.period_out();
        float acc = 0.0f;
        for (uint32_t t = 0; t < m_taps; t++){
            acc += table.tap_start(t)[wr] * input[base + t];
        }
        return {acc, base};
    };

    const double center = ((double)p0 - shift) / ratio;
    double max_diff = 0;
    size_t first_bad = 0;
    const double from = std::max(0.0, center - 32.0);
    const double to = center + 32.0;
    for (size_t c = (size_t)std::floor(from); c <= (size_t)std::ceil(to); c++){
        const auto got_base = out_sample(c);
        const double want = (got_base.second <= p0 && p0 < got_base.second + m_taps)
            ? weight_at((double)p0 - (double)c * ratio - shift)
            : 0.0;
        const double diff = std::fabs((double)got_base.first - want);
        if (diff > max_diff){
            max_diff = diff;
            first_bad = c;
        }
    }
    if (max_diff > 1e-6){
        return "Impulse response at output " + std::to_string(first_bad) +
            " differs by " + std::to_string(max_diff) +
            " from the windowed sinc centered on input time p0 evaluated at "
            "c * ratio + (taps - 1) / 2; the table does not reconstruct at "
            "time_shift()."
        ;
    }

    return true;
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
    //  1:1, upsample, downsample, strong downsampling and irrational
    //  ratios, with chunk sizes that stress the carry buffer on either
    //  side of the block size. The strong-downsampling combos stress the
    //  case where a block's window-start span exceeds the staged input.
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
        { 90000, 20000, 1, 4000, 250, 4, 9.0 },
        { 128000, 8000, 1, 4000, 250, 8, 9.0 },
        //  Odd tap count: on the unrolling kernels the final partial
        //  group reads the zero-padded tap row's input samples, which is
        //  what taps_allocated() in need() accounts for.
        { 44100, 22050, 1, 4000, 250, 3, 9.0 },
    };
    for (auto& combo : combos){
        database.add<ResamplerStreamTest>(
            combo.in_rate, combo.out_rate, combo.channels,
            combo.samples, combo.push, combo.taps, combo.beta
        );
    }

    //  A spread of tap counts across the supported range on the 1:1 combo
    //  (16 taps is the first combo above): counts below the kernels' tap
    //  blocks are zero padded, odd counts exercise the unrolls' final
    //  partial group, and the wide counts stress the carry sizing and the
    //  Kaiser window width.
    const uint32_t sweep_taps[] = { 1, 2, 3, 4, 5, 8, 32, 48, 63, 64 };
    for (uint32_t taps : sweep_taps){
        database.add<ResamplerStreamTest>(44100, 44100, 1, 4000, 100, taps, 9.0);
    }
    //  Degenerate and wide tap counts on the strongest upsampling combo,
    //  where the tap count dominates the next block's input requirement.
    database.add<ResamplerStreamTest>(8000, 44100, 1, 4000, 100, 1, 9.0);
    database.add<ResamplerStreamTest>(8000, 44100, 1, 4000, 100, 64, 9.0);

    //  Dispatch choices across rate ratios, with a spread of tap counts
    //  for the table itself. Odd tap counts exercise the unrolls' final
    //  partial group reading the table's zero padded rows.
    const struct{
        size_t in_rate;
        size_t out_rate;
        uint32_t taps;
    } dispatch_combos[] = {
        { 44100, 44100, 16 },
        { 44100, 48000, 16 },
        { 48000, 44100, 16 },
        { 44100, 88200, 16 },
        { 88200, 44100, 16 },
        { 96000, 44100, 16 },
        { 44100, 22050, 8 },
        { 90000, 20000, 4 },
        { 8000, 44100, 3 },
        //  The tap extremes across the kernel families.
        { 44100, 44100, 1 },
        { 44100, 48000, 64 },
        { 88200, 44100, 32 },
        { 128000, 8000, 64 },
    };
    for (auto& combo : dispatch_combos){
        database.add<BlockDispatchTest>(
            combo.in_rate, combo.out_rate, combo.taps
        );
    }

    //  The output time base is shifted (taps - 1) / 2 input samples past
    //  the input time base (the median-tap alignment). Verify the
    //  accessor's value and the actual alignment across ratios and tap
    //  parities.
    const struct{
        size_t in_rate;
        size_t out_rate;
        uint32_t taps;
    } timeshift_combos[] = {
        { 44100, 44100, 16 },
        { 44100, 44100, 31 },
        { 44100, 48000, 16 },
        { 48000, 44100, 31 },
        { 88200, 44100, 32 },
        { 44100, 88200, 3 },
        { 8000, 44100, 8 },
        { 128000, 8000, 64 },
    };
    for (auto& combo : timeshift_combos){
        database.add<WeightTableTimeShiftTest>(
            combo.in_rate, combo.out_rate, combo.taps
        );
    }

    //  The Default kernel doubles as the reference for the SIMD variants,
    //  with the tap extremes alongside the standard count.
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 1, 8, 1, 0
    );
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 16, 8, 1, 0
    );
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 64, 8, 1, 0
    );
    //  The block straddles the table period (147) into the continuing
    //  index columns.
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        48000, 44100, 16, 8, 1, 140
    );
    database.add<BlockKernelTest>(
        "Default", run_block8_Default,
        96000, 44100, 16, 8, 1, 0
    );

#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x64_SSE2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x64_FMA3(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_x64_AVX512(database);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        add_tests_ARM64_NEON(database);
    }
#endif
}



}
}
}
