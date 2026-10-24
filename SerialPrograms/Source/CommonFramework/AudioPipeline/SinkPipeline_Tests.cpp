/*  Audio Pipeline - Sink Gate Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Self-contained invariants for the pipeline tree's sink branch
 *  (RawAudio2WorkingAudio -> [resampler] -> [mixer] -> WorkingAudio2RawAudio).
 *  Everything runs on plain kernel objects, so no audio I/O is involved:
 *
 *   1. No-op cases (same rate, channel count, and sample format on both
 *      sides): the tree's raw output must be bit-identical to the input
 *      (float), or within 1 LSB of a plain-scalar reference (sint16).
 *
 *   2. Resampled / mixed cases: output length, tone amplitudes, and the
 *      upmix / downmix channel behavior.
 *
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <numbers>
#include <random>
#include <thread>
#include <vector>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QCoreApplication>
#include <QEventLoop>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/Backends/AudioPassthroughPairQt.h"
#include "CommonFramework/AudioPipeline/ChannelMixer.h"
#include "Kernels/AudioPipeline/Kernels_RawAudio2WorkingAudio.h"
#include "Kernels/AudioPipeline/Kernels_WorkingAudio2RawAudio.h"
#include "Kernels/AudioPipeline/Kernels_ChannelMixer.h"
#include "Kernels/AudioResampler/Kernels_AudioResampler.h"
#include "SinkPipeline_Tests.h"

namespace PokemonAutomation{

namespace{



Kernels::AudioPipeline::SampleFormat to_kernels_format(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:   return Kernels::AudioPipeline::SampleFormat::UINT8;
    case AudioSampleFormat::SINT16:  return Kernels::AudioPipeline::SampleFormat::SINT16;
    case AudioSampleFormat::SINT32:  return Kernels::AudioPipeline::SampleFormat::SINT32;
    case AudioSampleFormat::FLOAT32: return Kernels::AudioPipeline::SampleFormat::FLOAT32;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported audio sample format.");
    }
}



//  QAudioFormat with just a channel count and, for 1 and 2 channels, the
//  standard layout. More channels leave the config unknown so that
//  resolve_positions() falls back to the standard layout for the count.
//  (Same convention as AudioPassthroughPairQt's channel_format().)
QAudioFormat channel_format(size_t channels){
    QAudioFormat format;
    format.setChannelCount((int)channels);
    if (channels <= 2){
        format.setChannelConfig(
            channels == 1
                ? QAudioFormat::ChannelConfig::ChannelConfigMono
                : QAudioFormat::ChannelConfig::ChannelConfigStereo
        );
    }
    return format;
}



//  Two tones plus low-level noise. Deterministic: the same generator
//  state always produces the same signal, on every platform.
std::vector<float> make_noisy_tones(
    size_t n,
    size_t rate,
    double f0,
    double f1,
    std::mt19937& generator
){
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    std::vector<float> out(n);
    const double two_pi = 2.0 * std::numbers::pi;
    for (size_t i = 0; i < n; i++){
        out[i] = 0.35f * (float)std::sin(two_pi * f0 * i / rate)
            + 0.25f * (float)std::sin(two_pi * f1 * i / rate)
            + 0.01f * noise(generator);
    }
    return out;
}



//  A single pure tone.
std::vector<float> make_tone(size_t n, size_t rate, double freq, float amplitude){
    std::vector<float> out(n);
    const double two_pi = 2.0 * std::numbers::pi;
    for (size_t i = 0; i < n; i++){
        out[i] = amplitude * (float)std::sin(two_pi * freq * i / rate);
    }
    return out;
}



//  The raw interleaved bytes of a set of float channels.
std::vector<char> interleave_bytes(std::initializer_list<std::vector<float>> channels){
    const size_t ch = channels.size();
    const size_t n = (*channels.begin()).size();
    std::vector<const float*> ptrs(ch);
    size_t i = 0;
    for (const auto& c : channels){
        ptrs[i++] = c.data();
    }
    std::vector<char> out(n * ch * sizeof(float));
    for (size_t s = 0; s < n; s++){
        char* dst = out.data() + s * ch * sizeof(float);
        for (size_t c = 0; c < ch; c++){
            std::memcpy(dst, &ptrs[c][s], sizeof(float));
            dst += sizeof(float);
        }
    }
    return out;
}



//  (max - min) / 2 of one channel over [skip, end).
float measure_amplitude(const std::vector<float>& interleaved, size_t channels, size_t channel, size_t skip){
    const size_t frames = interleaved.size() / channels;
    if (frames <= skip){
        return 0.0f;
    }
    float lo = interleaved[skip * channels + channel];
    float hi = lo;
    for (size_t i = skip + 1; i < frames; i++){
        float v = interleaved[i * channels + channel];
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    return (hi - lo) * 0.5f;
}



//  Records the raw byte stream at the end of the tree chain.
class RecordingRawListener : public Kernels::AudioPipeline::RawAudioStreamListener{
public:
    RecordingRawListener(AudioSampleFormat format, size_t rate, size_t channels)
        : Kernels::AudioPipeline::RawAudioStreamListener(to_kernels_format(format), rate, channels)
        , m_frame_bytes(sample_size(format) * channels)
    {}
    virtual void push_samples(size_t frames, const void* data) override{
        const char* bytes = (const char*)data;
        const size_t count = frames * m_frame_bytes;
        m_bytes.insert(m_bytes.end(), bytes, bytes + count);
    }
    const std::vector<char>& bytes() const{
        return m_bytes;
    }

private:
    const size_t m_frame_bytes;
    std::vector<char> m_bytes;
};



//  Tree path: raw bytes -> RawAudio2WorkingAudio -> [resampler] -> [mixer]
//  -> WorkingAudio2RawAudio, mirroring AudioPassthroughPairQt::
//  rebuild_sink_branch().
std::vector<char> run_tree(
    Logger& logger,
    const std::vector<char>& raw,
    AudioSampleFormat in_format, size_t in_rate, size_t in_channels,
    AudioSampleFormat out_format, size_t out_rate, size_t out_channels
){
    Kernels::AudioPipeline::RawAudio2WorkingAudio decoder(
        to_kernels_format(in_format), in_rate, in_channels, 1.0f
    );
    Kernels::AudioPipeline::WorkingAudio2RawAudio encoder(
        to_kernels_format(out_format), out_rate, out_channels, 1.0f
    );
    RecordingRawListener out(out_format, out_rate, out_channels);
    encoder.add_listener(out);

    std::unique_ptr<Kernels::AudioResampler::AudioResampler> resampler;
    std::unique_ptr<Kernels::AudioPipeline::ChannelMixer> mixer;
    Kernels::AudioPipeline::WorkingAudioStreamListener* head = &encoder;
    if (in_rate != out_rate){
        resampler.reset(
            new Kernels::AudioResampler::AudioResampler(
                logger,
                in_rate,
                out_rate,
                out_channels
            )
        );
        resampler->add_listener(*head);
        head = resampler.get();
    }
    if (in_channels != out_channels){
        std::vector<float> matrix = build_channel_mixer_matrix(
            channel_format(in_channels), channel_format(out_channels)
        );
        if (matrix.size() != in_channels * out_channels){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No channel mixer matrix.");
        }
        mixer.reset(new Kernels::AudioPipeline::ChannelMixer(
            logger, in_rate, in_channels, out_channels, std::move(matrix)
        ));
        mixer->add_listener(*head);
        head = mixer.get();
    }
    decoder.add_listener(*head);

    //  Feed in odd-sized chunks. (The nodes die child-first with scope
    //  exit, after the feed is done, like the pair's teardown.)
    const size_t frame_size = sample_size(in_format) * in_channels;
    const size_t frames_total = raw.size() / frame_size;
    size_t done = 0;
    while (done < frames_total){
        size_t chunk = std::min((size_t)997, frames_total - done);
        decoder.push_samples(chunk, raw.data() + done * frame_size);
        done += chunk;
    }

    return out.bytes();
}



//
//  No-op case: 2ch x 48000 FLOAT32 in, same out. The tree must pass the
//  raw bytes through bit-identically.
//
class SinkGateDual48000Test : public UnitTest{
public:
    SinkGateDual48000Test()
        : UnitTest("AudioPipeline - SinkGate_Dual48000_BitExact")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 8192;

        std::mt19937 generator(42);
        std::vector<float> l = make_noisy_tones(n, rate, 4031.25, 14062.5, generator);
        std::vector<float> r = make_noisy_tones(n, rate, 4031.25, 14062.5, generator);
        std::vector<char> raw = interleave_bytes({l, r});

        std::vector<char> tree = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, rate, 2,
            AudioSampleFormat::FLOAT32, rate, 2
        );

        if (tree.size() != raw.size()){
            return "Length mismatch: raw = " + std::to_string(raw.size())
                + ", tree = " + std::to_string(tree.size()) + " bytes.";
        }
        if (std::memcmp(raw.data(), tree.data(), tree.size()) != 0){
            for (size_t c = 0; c < tree.size(); c++){
                if (raw[c] != tree[c]){
                    return "First differing byte at " + std::to_string(c)
                        + ": raw = " + tostr_default((int)raw[c])
                        + ", tree = " + tostr_default((int)tree[c]) + ".";
                }
            }
        }

        return true;
    }
};



//
//  No-op case: 1ch x 48000 FLOAT32 in, same out. Bit-identical pass-
//  through, as the dual case above.
//
class SinkGateMono48000Test : public UnitTest{
public:
    SinkGateMono48000Test()
        : UnitTest("AudioPipeline - SinkGate_Mono48000_BitExact")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 8192;

        std::mt19937 generator(42);
        std::vector<float> m = make_noisy_tones(n, rate, 4031.25, 14062.5, generator);
        std::vector<char> raw = interleave_bytes({m});

        std::vector<char> tree = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, rate, 1,
            AudioSampleFormat::FLOAT32, rate, 1
        );

        if (tree.size() != raw.size()){
            return "Length mismatch: raw = " + std::to_string(raw.size())
                + ", tree = " + std::to_string(tree.size()) + " bytes.";
        }
        if (std::memcmp(raw.data(), tree.data(), tree.size()) != 0){
            for (size_t c = 0; c < tree.size(); c++){
                if (raw[c] != tree[c]){
                    return "First differing byte at " + std::to_string(c)
                        + ": raw = " + tostr_default((int)raw[c])
                        + ", tree = " + tostr_default((int)tree[c]) + ".";
                }
            }
        }

        return true;
    }
};



//
//  No-op shape, different sample format: FLOAT32 in, SINT16 out, same
//  rate / channels. Every converter variant (scalar or SIMD) must agree
//  with the plain-scalar reference — x * 32768, clamped to int16 —
//  within 1 LSB per sample (rounding / clamping policy).
//
class SinkGateSint16NoOpTest : public UnitTest{
public:
    SinkGateSint16NoOpTest()
        : UnitTest("AudioPipeline - SinkGate_Sint16NoOp_1LSB")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 8192;

        std::mt19937 generator(42);
        std::vector<float> l = make_noisy_tones(n, rate, 4031.25, 14062.5, generator);
        std::vector<float> r = make_noisy_tones(n, rate, 4031.25, 14062.5, generator);
        std::vector<char> raw = interleave_bytes({l, r});

        std::vector<char> tree = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, rate, 2,
            AudioSampleFormat::SINT16, rate, 2
        );
        if (tree.size() != raw.size() / 2){
            return "Length mismatch: raw = " + std::to_string(raw.size())
                + ", tree = " + std::to_string(tree.size()) + " bytes.";
        }

        std::vector<int16_t> tree_samples(tree.size() / sizeof(int16_t));
        std::memcpy(tree_samples.data(), tree.data(), tree.size());

        for (size_t c = 0; c < tree_samples.size(); c++){
            const float x = (c % 2 == 0) ? l[c / 2] : r[c / 2];
            float v = x * 32768.0f;
            v = std::clamp(v, -32768.0f, 32767.0f);
            const int ref = (int)std::lround((double)v);
            if (std::abs((int)tree_samples[c] - ref) > 1){
                return "Sample " + std::to_string(c) + " differs from the scalar reference by more than 1 LSB: reference = "
                    + std::to_string(ref) + ", tree = " + std::to_string(tree_samples[c]) + ".";
            }
        }

        return true;
    }
};



//
//  Resample up: 2ch x 44100 -> 2ch x 48000 FLOAT32. The legacy path has
//  no equivalent (it opened the device at the input rate), so verify the
//  tree's output on its own: the right number of samples and each input
//  tone at its input amplitude.
//
class SinkGateResampleUpTest : public UnitTest{
public:
    SinkGateResampleUpTest()
        : UnitTest("AudioPipeline - SinkGate_ResampleUp_44100To48000")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t in_rate = 44100;
        const size_t out_rate = 48000;
        //  Long stream: the resampler holds back a tail carry (less than a
        //  kernel block; up to ~128 outputs on x86 AVX-512) that is never
        //  flushed, so the relative slack must dwarf that.
        const size_t n = 65536;

        std::vector<float> l = make_tone(n, in_rate, 4031.25, 0.5f);
        std::vector<float> r = make_tone(n, in_rate, 14062.5, 0.4f);
        std::vector<char> raw = interleave_bytes({l, r});

        std::vector<char> out = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, in_rate, 2,
            AudioSampleFormat::FLOAT32, out_rate, 2
        );
        const size_t out_samples = out.size() / sizeof(float);
        const double expected = (double)n * 2.0 * ((double)out_rate / (double)in_rate);
        if (out_samples < expected * 0.98 || out_samples > expected * 1.02){
            return "Output length out of range: " + std::to_string(out_samples)
                + " samples, expected ~" + std::to_string((size_t)expected) + ".";
        }

        std::vector<float> out_floats(out_samples);
        std::memcpy(out_floats.data(), out.data(), out.size());

        //  Skip a few leading samples for the resampler transient.
        const size_t skip = 512;
        float amplitude_l = measure_amplitude(out_floats, 2, 0, skip);
        float amplitude_r = measure_amplitude(out_floats, 2, 1, skip);
        if (std::abs(amplitude_l - 0.5f) > 0.025f){
            return "Left tone amplitude out of range: " + tostr_default(amplitude_l) + ", expected ~0.5.";
        }
        if (std::abs(amplitude_r - 0.4f) > 0.02f){
            return "Right tone amplitude out of range: " + tostr_default(amplitude_r) + ", expected ~0.4.";
        }

        return true;
    }
};



//
//  Decimate: 1ch x 96000 -> 1ch x 48000 FLOAT32.
//
class SinkGateDecimateTest : public UnitTest{
public:
    SinkGateDecimateTest()
        : UnitTest("AudioPipeline - SinkGate_Decimate_96000To48000")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t in_rate = 96000;
        const size_t out_rate = 48000;
        //  Long stream: see the note in SinkGateResampleUpTest about the
        //  unflushed tail carry.
        const size_t n = 65536;

        std::vector<float> m = make_tone(n, in_rate, 4031.25, 0.5f);
        std::vector<char> raw = interleave_bytes({m});

        std::vector<char> out = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, in_rate, 1,
            AudioSampleFormat::FLOAT32, out_rate, 1
        );
        const size_t out_samples = out.size() / sizeof(float);
        const double expected = (double)n * ((double)out_rate / (double)in_rate);
        if (out_samples < expected * 0.99 || out_samples > expected * 1.01){
            return "Output length out of range: " + std::to_string(out_samples)
                + " samples, expected ~" + std::to_string((size_t)expected) + ".";
        }

        std::vector<float> out_floats(out_samples);
        std::memcpy(out_floats.data(), out.data(), out.size());

        const size_t skip = 512;
        float amplitude = measure_amplitude(out_floats, 1, 0, skip);
        if (std::abs(amplitude - 0.5f) > 0.015f){
            return "Tone amplitude out of range: " + tostr_default(amplitude) + ", expected ~0.5.";
        }

        return true;
    }
};



//
//  Upmix: 1ch x 48000 -> 2ch x 48000 FLOAT32. The mixer's mono -> stereo
//  matrix copies the channel to both sides at gain 1, so the two output
//  channels must be bit-identical with the input amplitude.
//
class SinkGateUpmixTest : public UnitTest{
public:
    SinkGateUpmixTest()
        : UnitTest("AudioPipeline - SinkGate_Upmix_MonoToStereo")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 8192;

        std::vector<float> m = make_tone(n, rate, 4031.25, 0.5f);
        std::vector<char> raw = interleave_bytes({m});

        std::vector<char> out = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, rate, 1,
            AudioSampleFormat::FLOAT32, rate, 2
        );
        if (out.size() != raw.size() * 2){
            return "Output length mismatch: " + std::to_string(out.size())
                + " bytes, expected " + std::to_string(raw.size() * 2) + ".";
        }

        std::vector<float> out_floats(out.size() / sizeof(float));
        std::memcpy(out_floats.data(), out.data(), out.size());

        for (size_t i = 0; i < n; i++){
            if (out_floats[2 * i + 0] != out_floats[2 * i + 1]){
                return "Channels differ at frame " + std::to_string(i) + ": L = "
                    + tostr_default(out_floats[2 * i + 0])
                    + ", R = " + tostr_default(out_floats[2 * i + 1]) + ".";
            }
        }
        float amplitude = measure_amplitude(out_floats, 2, 0, 0);
        if (std::abs(amplitude - 0.5f) > 0.005f){
            return "Tone amplitude out of range: " + tostr_default(amplitude) + ", expected ~0.5.";
        }

        return true;
    }
};



//
//  Downmix: 2ch x 48000 -> 1ch x 48000 FLOAT32. The mixer's stereo ->
//  mono matrix is a 0.5 / 0.5 average, so both input tones survive at
//  their input amplitudes.
//
class SinkGateDownmixTest : public UnitTest{
public:
    SinkGateDownmixTest()
        : UnitTest("AudioPipeline - SinkGate_Downmix_StereoToMono")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 8192;

        std::vector<float> l = make_tone(n, rate, 4031.25, 0.5f);
        std::vector<float> r = make_tone(n, rate, 14062.5, 0.4f);
        std::vector<char> raw = interleave_bytes({l, r});

        std::vector<char> out = run_tree(
            logger, raw,
            AudioSampleFormat::FLOAT32, rate, 2,
            AudioSampleFormat::FLOAT32, rate, 1
        );
        if (out.size() != raw.size() / 2){
            return "Output length mismatch: " + std::to_string(out.size())
                + " bytes, expected " + std::to_string(raw.size() / 2) + ".";
        }

        std::vector<float> out_floats(out.size() / sizeof(float));
        std::memcpy(out_floats.data(), out.data(), out.size());

        //  Both tones are present in the mix; check the combined
        //  amplitude against the two-tone reference measured the same way.
        std::vector<float> reference(n);
        for (size_t i = 0; i < n; i++){
            reference[i] = l[i] * 0.5f + r[i] * 0.5f;
        }
        float out_amplitude = measure_amplitude(out_floats, 1, 0, 0);
        float ref_amplitude = measure_amplitude(reference, 1, 0, 0);
        if (ref_amplitude <= 0.0f){
            return "Reference amplitude is not positive (" + tostr_default(ref_amplitude) + ").";
        }
        double ratio = out_amplitude / ref_amplitude;
        if (ratio < 0.95 || ratio > 1.05){
            return "Output / reference amplitude ratio out of range: " + tostr_default((float)ratio) + ".";
        }

        return true;
    }
};



//
//  Multichannel (5.1) file input: 6ch x 48000 in, 1ch / 2ch out, same
//  Regression gate for the mp3-probe deadlock: the file source's
//  constructor probes the decoder on a nested event loop, and events
//  queued on the pair's thread are dispatched during that loop. A
//  queued set_audio_sink must be able to take the pair's lock then, so
//  the source has to be constructed outside the lock (a regression
//  here deadlocks; the watchdog aborts the run instead of hanging it).
class SinkGateFileSourceNoDeadlockTest : public UnitTest{
public:
    SinkGateFileSourceNoDeadlockTest()
        : UnitTest("AudioPipeline - SinkGate_FileSource_NoDeadlock")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        const std::string fixture =
            UNIT_TEST_RESOURCE_PATH() + "CommonFramework/AudioPipeline/decoded_probe_44100_2ch.mp3";
        if (!std::filesystem::exists(std::filesystem::path(fixture))){
            return "Missing mp3 fixture: " + fixture + ".";
        }
        if (!QAudioDecoder(nullptr).isSupported()){
            return UnitTestResult(
                UnitTestResult::SKIPPED,
                "No audio decode backend (Qt Multimedia media plugin missing or failing to load)."
            );
        }

        std::atomic<bool> finished{false};
        std::thread watchdog([&finished]{
            for (int i = 0; i < 300 && !finished.load(std::memory_order_relaxed); i++){
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (!finished.load(std::memory_order_relaxed)){
                std::fprintf(stderr, "SinkGate_FileSource_NoDeadlock: still running after 30 s: deadlock, aborting.\n");
                std::abort();
            }
        });

        AudioPassthroughPairQt pair(logger);
        QMetaObject::invokeMethod(
            &pair,
            [&pair]{
                pair.set_audio_sink(AudioDeviceInfo(), 1.0);
            },
            Qt::QueuedConnection
        );
        //  Must complete: the probe runs before the lock is taken, so the
        //  queued sink event above can run through while it decodes.
        pair.set_audio_source(fixture);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 500);

        finished.store(true, std::memory_order_relaxed);
        watchdog.join();
        return true;
    }
};



//  rate, so the tree is a plain mix. The input format carries no channel
//  config, so the matrix must come from the standard 5.1 fallback layout
//  (resolve_positions), and the mix must match a scalar reference
//  computed from the same matrix. This also pins that a 6ch input gets a
//  real (non-empty) matrix at all: before channel_format() stopped
//  forcing the stereo config on counts above 2, the 6ch shape resolved
//  as 2ch and no matrix could be built.
//
class SinkGateMultichannel51Test : public UnitTest{
public:
    SinkGateMultichannel51Test()
        : UnitTest("AudioPipeline - SinkGate_Multichannel51_Downmix")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        //  6ch (5.1: FL FR C LFE BL BR) x 48000 float, one constant per
        //  channel.
        const size_t n = 4800;
        std::vector<float> fl(n, 0.3f), fr(n, 0.1f), c(n, -0.2f), lfe(n, 0.4f), bl(n, 0.05f), br(n, -0.05f);
        const std::vector<std::vector<float>> in_channels = {fl, fr, c, lfe, bl, br};
        const std::vector<char> raw = interleave_bytes({fl, fr, c, lfe, bl, br});

        for (size_t out_channels : {1, 2}){
            std::vector<float> matrix = build_channel_mixer_matrix(
                channel_format(6), channel_format(out_channels)
            );
            if (matrix.size() != 6 * out_channels){
                return "Expected a 6x" + std::to_string(out_channels) + " mix matrix, got "
                    + std::to_string(matrix.size()) + " entries.";
            }

            //  The LFE channel does not reach a mono mix.
            if (out_channels == 1 && matrix[3] != 0.0f){
                return "The LFE channel should have zero gain into mono.";
            }

            const std::vector<char> out = run_tree(
                logger, raw,
                AudioSampleFormat::FLOAT32, 48000, 6,
                AudioSampleFormat::FLOAT32, 48000, out_channels
            );
            const size_t out_frames = out.size() / (sizeof(float) * out_channels);
            if (out_frames != n){
                return "Expected " + std::to_string(n) + " output frames, got "
                    + std::to_string(out_frames) + ".";
            }
            for (size_t i = 0; i < n; i++){
                for (size_t o = 0; o < out_channels; o++){
                    float reference = 0.0f;
                    for (size_t k = 0; k < 6; k++){
                        reference += matrix[k + 6 * o] * in_channels[k][i];
                    }
                    const float got = *reinterpret_cast<const float*>(
                        out.data() + (i * out_channels + o) * sizeof(float)
                    );
                    if (std::fabs(got - reference) > 1e-6f){
                        return "Mix mismatch on output channel " + std::to_string(o)
                            + " frame " + std::to_string(i) + ": got "
                            + tostr_default(got) + ", expected " + tostr_default(reference) + ".";
                    }
                }
            }
        }

        return true;
    }
};



}



void add_tests_SinkPipeline(UnitTestDatabase& database){
    database.add<SinkGateDual48000Test>();
    database.add<SinkGateMono48000Test>();
    database.add<SinkGateSint16NoOpTest>();
    database.add<SinkGateResampleUpTest>();
    database.add<SinkGateDecimateTest>();
    database.add<SinkGateUpmixTest>();
    database.add<SinkGateDownmixTest>();
    database.add<SinkGateMultichannel51Test>();
    database.add<SinkGateFileSourceNoDeadlockTest>();
}



}
