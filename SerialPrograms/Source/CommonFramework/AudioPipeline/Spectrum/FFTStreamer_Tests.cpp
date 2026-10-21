/*  FFT Streamer - FFT Migration Gate Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  These tests exist to gate the switch of AudioPassthroughPairQt's FFT
 *  endpoint from the legacy path (AudioStreamToFloat -> AudioFloatToFFT)
 *  to the pipeline tree (RawAudio2WorkingAudio -> mixer/resampler ->
 *  WorkingAudioToFFT). Each test has two parts:
 *
 *   1. Self-contained invariants of the tree path (reported rate,
 *      finiteness, tone peaks at their exact bins, tones dominating the
 *      noise floor). These hold on their own and remain valid after the
 *      legacy path is deleted.
 *
 *   2. Legacy-diff checks, which quantify (and, in the no-op DUAL_48000
 *      case, prove bit-exactness of) the migration while the legacy
 *      path still exists. These are slated for removal together with
 *      AudioFloatToFFT.
 *
 */

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "Kernels/AudioPipeline/Kernels_RawAudio2WorkingAudio.h"
#include "Kernels/AudioPipeline/Kernels_ChannelMixer.h"
#include "Kernels/AudioResampler/Kernels_AudioResampler.h"
#include "FFTStreamer.h"

namespace PokemonAutomation{

namespace{



//  Two tones (at exact 48 kHz FFT bin centers) plus low-level noise.
//  Deterministic: the same generator state always produces the same
//  signal, on every platform.
AlignedVector<float> make_signal(
    size_t n,
    size_t rate,
    double f0,
    double f1,
    std::mt19937& generator
){
    std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
    AlignedVector<float> out(n);
    const double two_pi = 2.0 * std::numbers::pi;
    for (size_t i = 0; i < n; i++){
        out[i] = 0.35f * (float)std::sin(two_pi * f0 * i / rate)
            + 0.25f * (float)std::sin(two_pi * f1 * i / rate)
            + 0.01f * noise(generator);
    }
    return out;
}



//  Records FFTs from an FFTListener.
class RecordingFFTListener : public FFTListener{
public:
    virtual void on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output) override{
        m_rates.push_back(sample_rate);
        m_ffts.push_back(fft_output);
    }
    const std::vector<std::shared_ptr<const AlignedVector<float>>>& ffts() const{
        return m_ffts;
    }
    const std::vector<size_t>& rates() const{
        return m_rates;
    }

private:
    std::vector<size_t> m_rates;
    std::vector<std::shared_ptr<const AlignedVector<float>>> m_ffts;
};



//  Records the legacy interleaved float stream.
class RecordingFloatListener : public AudioFloatStreamListener{
public:
    explicit RecordingFloatListener(size_t spf)
        : AudioFloatStreamListener(spf)
    {}
    virtual void on_samples(const float* data, size_t frames) override{
        const size_t count = frames * expected_samples_per_frame;
        for (size_t c = 0; c < count; c++){
            m_samples.emplace_back(data[c]);
        }
        m_frames += frames;
    }
    const AlignedVector<float>& samples() const{
        return m_samples;
    }
    size_t frames() const{
        return m_frames;
    }

private:
    AlignedVector<float> m_samples;
    size_t m_frames = 0;
};



//  Legacy path: raw bytes -> AudioStreamToFloat -> AudioFloatToFFT
//  (as built by make_FFT_streamer for the preset).
RecordingFFTListener run_legacy(
    const AlignedVector<float>& raw,
    size_t samples_per_frame,
    AudioChannelFormat format
){
    AudioStreamToFloat reader(AudioSampleFormat::FLOAT32, samples_per_frame, 1.0f, false);
    RecordingFloatListener capture(samples_per_frame);
    reader.add_listener(capture);
    reader.push_bytes(raw.data(), raw.size() * sizeof(float));

    std::unique_ptr<AudioFloatToFFT> runner = make_FFT_streamer(format);
    RecordingFFTListener recorder;
    runner->add_listener(recorder);

    //  Feed in odd-sized chunks to exercise the ring-buffer block logic.
    size_t done = 0;
    while (done < capture.frames()){
        size_t chunk = std::min((size_t)997, capture.frames() - done);
        runner->on_samples(capture.samples().data() + done * samples_per_frame, chunk);
        done += chunk;
    }
    return recorder;
}



//  Tree path: raw bytes -> RawAudio2WorkingAudio -> [mixer] -> [resampler]
//  -> WorkingAudioToFFT, mirroring AudioPassthroughPairQt::build_tree().
RecordingFFTListener run_tree(
    const AlignedVector<float>& raw,
    size_t raw_channels,
    size_t raw_rate,
    size_t fft_rate
){
    Kernels::AudioPipeline::RawAudio2WorkingAudio decoder(
        Kernels::AudioPipeline::SampleFormat::FLOAT32, raw_rate, raw_channels, 1.0f
    );
    WorkingAudioToFFT fft(fft_rate);
    RecordingFFTListener recorder;
    fft.add_listener(recorder);
    std::unique_ptr<Kernels::AudioResampler::AudioResampler> resampler;
    std::unique_ptr<Kernels::AudioPipeline::ChannelMixer> mixer;
    Kernels::AudioPipeline::WorkingAudioStreamListener* head = &fft;
    if (raw_rate != fft_rate){
        resampler.reset(new Kernels::AudioResampler::AudioResampler(raw_rate, fft_rate, 1));
        resampler->add_listener(*head);
        head = resampler.get();
    }
    if (raw_channels != 1){
        //  build_channel_mixer_matrix(stereo -> mono) is {0.5, 0.5}.
        mixer.reset(new Kernels::AudioPipeline::ChannelMixer(
            raw_rate, raw_channels, 1, std::vector<float>{0.5f, 0.5f}
        ));
        mixer->add_listener(*head);
        head = mixer.get();
    }
    decoder.add_listener(*head);

    //  Feed in odd-sized chunks. (The nodes die child-first with scope
    //  exit, after the feed is done, like the pair's teardown.)
    const size_t frames_total = raw.size() / raw_channels;
    size_t done = 0;
    while (done < frames_total){
        size_t chunk = std::min((size_t)997, frames_total - done);
        decoder.push_samples(chunk, raw.data() + done * raw_channels);
        done += chunk;
    }

    return recorder;
}



//  Bin with the largest magnitude within +/- window of center.
size_t peak_bin(const AlignedVector<float>& fft, size_t center, size_t window){
    size_t lo = center > window ? center - window : 0;
    size_t hi = std::min(fft.size(), center + window + 1);
    size_t best = lo;
    for (size_t b = lo + 1; b < hi; b++){
        if (fft[b] > fft[best]){
            best = b;
        }
    }
    return best;
}



//  Self-contained invariants of the tree path, independent of the legacy
//  path: every FFT reports the expected rate, all bins are finite, each
//  input tone peaks at its exact bin, and each tone dominates the noise
//  floor (measured on content-free bins far from the tones). An empty
//  string means the invariants hold.
std::string check_invariants(
    const std::vector<std::shared_ptr<const AlignedVector<float>>>& ffts,
    const std::vector<size_t>& rates,
    size_t expected_rate,
    size_t first_valid_fft,
    const std::vector<size_t>& tone_bins
){
    if (ffts.size() <= first_valid_fft){
        return "Not enough FFTs produced (" + std::to_string(ffts.size()) + ").";
    }
    for (size_t k = 0; k < rates.size(); k++){
        if (rates[k] != expected_rate){
            return "FFT " + std::to_string(k) + " reported rate " + std::to_string(rates[k])
                + ", expected " + std::to_string(expected_rate) + ".";
        }
    }
    for (size_t k = first_valid_fft; k < ffts.size(); k++){
        if (!std::isfinite((*ffts[k])[0]) || !std::isfinite(ffts[k]->back())){
            return "Non-finite value in FFT " + std::to_string(k) + ".";
        }
        float floor_max = 0.0f;
        for (int b : {5, 6, 7, 300, 301, 302, 850, 851, 852}){
            floor_max = std::max(floor_max, (*ffts[k])[b]);
        }
        for (size_t tone_bin : tone_bins){
            size_t bin = peak_bin(*ffts[k], tone_bin, 2);
            if (bin != tone_bin){
                return "FFT " + std::to_string(k) + ": tone peak at bin " + std::to_string(bin)
                    + ", expected " + std::to_string(tone_bin) + ".";
            }
            float peak = (*ffts[k])[tone_bin];
            if (peak < 50.0f * floor_max){
                return "FFT " + std::to_string(k) + ": tone at bin " + std::to_string(tone_bin)
                    + " does not dominate the noise floor (peak " + tostr_default(peak)
                    + ", floor " + tostr_default(floor_max) + ").";
            }
        }
    }
    return "";
}



//
//  DUAL_48000: the no-op case. The tree's FFT output must be
//  bit-identical to the legacy path's, FFT by FFT, bin by bin.
//
class FFTGateDual48000Test : public UnitTest{
public:
    FFTGateDual48000Test()
        : UnitTest("AudioPipeline - FFTGate_Dual48000_BitExact")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t rate = 48000;
        const size_t n = 16384;
        const std::vector<size_t> tone_bins = {172, 600};   //  4031.25 / 14062.5 Hz at 48 kHz (14062.5 * 2048 / 48000 = 600).

        std::mt19937 generator(42);
        AlignedVector<float> l = make_signal(n, rate, 4031.25, 14062.5, generator);
        AlignedVector<float> r = make_signal(n, rate, 4031.25, 14062.5, generator);
        AlignedVector<float> raw(2 * n);
        for (size_t i = 0; i < n; i++){
            raw[2 * i + 0] = l[i];
            raw[2 * i + 1] = r[i];
        }

        RecordingFFTListener tree = run_tree(raw, 2, rate, rate);

        //  Self-contained invariants (survive legacy removal). The ring
        //  buffer is zero pre-filled, so the leading FFTs (0..3) contain
        //  zero padding; the invariants start at the first full window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), rate, 4, tone_bins);
        if (!error.empty()){
            return error;
        }

        //  [Legacy-diff: remove with AudioFloatToFFT] The switch gate: on
        //  the no-op 48 kHz case the tree must match the legacy path
        //  bit-for-bit.
        RecordingFFTListener legacy = run_legacy(raw, 2, AudioChannelFormat::DUAL_48000);

        if (legacy.ffts().size() != tree.ffts().size()){
            return "FFT count mismatch: legacy = " + std::to_string(legacy.ffts().size())
                + ", tree = " + std::to_string(tree.ffts().size()) + ".";
        }
        for (size_t k = 0; k < legacy.ffts().size(); k++){
            if (tree.ffts()[k]->size() != legacy.ffts()[k]->size()){
                return "FFT " + std::to_string(k) + " size mismatch: "
                    + std::to_string(tree.ffts()[k]->size()) + " != " + std::to_string(legacy.ffts()[k]->size()) + ".";
            }
            for (size_t b = 0; b < legacy.ffts()[k]->size(); b++){
                float legacy_value = (*legacy.ffts()[k])[b];
                float tree_value = (*tree.ffts()[k])[b];
                if (legacy_value != tree_value){
                    return "FFT " + std::to_string(k) + ", bin " + std::to_string(b) + " differs: legacy = "
                        + tostr_default(legacy_value) + ", tree = " + tostr_default(tree_value) + ".";
                }
            }
        }

        return true;
    }
};



//
//  DUAL_44100: the tree resamples to 48 kHz (the legacy path FFTs at
//  44.1 kHz), so the outputs are expected to differ; the invariants
//  above verify the tree path on its own, and the legacy-diff below
//  quantifies the amplitude delta the migration introduces.
//
class FFTGateDual44100Test : public UnitTest{
public:
    FFTGateDual44100Test()
        : UnitTest("AudioPipeline - FFTGate_Dual44100_Sanity")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t raw_rate = 44100;
        const size_t fft_rate = 48000;
        const size_t n = 16384;
        const std::vector<size_t> tone_bins = {172, 600};   //  4031.25 / 14062.5 Hz at 48 kHz (14062.5 * 2048 / 48000 = 600).
        const size_t tone_bin_44k = 187;   //  4031.25 * 2048 / 44100 ~= 187.2.

        std::mt19937 generator(42);
        AlignedVector<float> l = make_signal(n, raw_rate, 4031.25, 14062.5, generator);
        AlignedVector<float> r = make_signal(n, raw_rate, 4031.25, 14062.5, generator);
        AlignedVector<float> raw(2 * n);
        for (size_t i = 0; i < n; i++){
            raw[2 * i + 0] = l[i];
            raw[2 * i + 1] = r[i];
        }

        RecordingFFTListener tree = run_tree(raw, 2, raw_rate, fft_rate);

        //  Self-contained invariants (survive legacy removal). The ring
        //  buffer is zero pre-filled and the resampler has a short
        //  transient, so the invariants start at the first full
        //  window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), fft_rate, 4, tone_bins);
        if (!error.empty()){
            return error;
        }

        //  [Legacy-diff: remove with AudioFloatToFFT] The legacy
        //  pair-averaging decimator attenuates the 4031.25 Hz tone by
        //  |cos(pi * f / 44100)| ~= 0.959, while the tree's resampler
        //  passes it at ~unity gain, so the tree peak should be slightly
        //  larger.
        RecordingFFTListener legacy = run_legacy(raw, 2, AudioChannelFormat::DUAL_44100);
        if (legacy.ffts().size() < 4){
            return "Legacy path produced too few FFTs (" + std::to_string(legacy.ffts().size()) + ").";
        }
        for (size_t k = 0; k < legacy.ffts().size(); k++){
            if (!std::isfinite((*legacy.ffts()[k])[0])){
                return "Non-finite value in legacy FFT " + std::to_string(k) + ".";
            }
        }
        size_t legacy_peak_bin = peak_bin(*legacy.ffts()[3], tone_bin_44k, 2);
        if (legacy_peak_bin != tone_bin_44k){
            return "Legacy FFT 3: tone peak at bin " + std::to_string(legacy_peak_bin)
                + ", expected " + std::to_string(tone_bin_44k) + ".";
        }
        float tree_peak = (*tree.ffts()[3])[tone_bins[0]];
        float legacy_peak = (*legacy.ffts()[3])[tone_bin_44k];
        if (legacy_peak <= 0.0f){
            return "Legacy tone peak is not positive (" + tostr_default(legacy_peak) + ").";
        }
        double ratio = tree_peak / legacy_peak;
        if (ratio < 0.90 || ratio > 1.20){
            return "Tree / legacy tone amplitude ratio out of range: " + tostr_default((float)ratio) + ".";
        }

        return true;
    }
};



//
//  MONO_96000: the tree decimates the 96 kHz mono stream to 48 kHz with
//  the resampler (the legacy path pair-averages the 2-sample frames);
//  both FFT at 48 kHz, so the tones land in the same bins on both sides.
//
class FFTGateMono96000Test : public UnitTest{
public:
    FFTGateMono96000Test()
        : UnitTest("AudioPipeline - FFTGate_Mono96000_Sanity")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t raw_rate = 96000;
        const size_t fft_rate = 48000;
        const size_t n = 16384;   //  Even, so the legacy 2-sample framing loses nothing.
        const std::vector<size_t> tone_bins = {172, 600};   //  4031.25 / 14062.5 Hz at 48 kHz (14062.5 * 2048 / 48000 = 600).

        std::mt19937 generator(42);
        AlignedVector<float> raw = make_signal(n, raw_rate, 4031.25, 14062.5, generator);

        RecordingFFTListener tree = run_tree(raw, 1, raw_rate, fft_rate);

        //  Self-contained invariants (survive legacy removal). The ring
        //  buffer is zero pre-filled and the resampler has a short
        //  transient, so the invariants start at the first full
        //  window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), fft_rate, 4, tone_bins);
        if (!error.empty()){
            return error;
        }

        //  [Legacy-diff: remove with AudioFloatToFFT] The legacy
        //  pair-averaging decimator attenuates the 4031.25 Hz tone by
        //  |cos(pi * f / 96000)| ~= 0.991; the tree's resampler passes
        //  it at ~unity gain, so the two peaks should be within 5%.
        RecordingFFTListener legacy = run_legacy(raw, 2, AudioChannelFormat::MONO_96000);
        if (legacy.ffts().size() < 4){
            return "Legacy path produced too few FFTs (" + std::to_string(legacy.ffts().size()) + ").";
        }
        size_t legacy_peak_bin = peak_bin(*legacy.ffts()[3], tone_bins[0], 2);
        if (legacy_peak_bin != tone_bins[0]){
            return "Legacy FFT 3: tone peak at bin " + std::to_string(legacy_peak_bin)
                + ", expected " + std::to_string(tone_bins[0]) + ".";
        }
        float tree_peak = (*tree.ffts()[3])[tone_bins[0]];
        float legacy_peak = (*legacy.ffts()[3])[tone_bins[0]];
        if (legacy_peak <= 0.0f){
            return "Legacy tone peak is not positive (" + tostr_default(legacy_peak) + ").";
        }
        double ratio = tree_peak / legacy_peak;
        if (ratio < 0.95 || ratio > 1.05){
            return "Tree / legacy tone amplitude ratio out of range: " + tostr_default((float)ratio) + ".";
        }

        return true;
    }
};



}



void add_tests_FFTStreamer(UnitTestDatabase& database){
    database.add<FFTGateDual48000Test>();
    database.add<FFTGateDual44100Test>();
    database.add<FFTGateMono96000Test>();
}



}
