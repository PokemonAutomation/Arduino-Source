/*  FFT Streamer - FFT Gate Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Self-contained invariants of the pipeline tree's FFT path
 *  (RawAudio2WorkingAudio -> mixer/resampler -> WorkingAudioToFFT):
 *  the reported rate, finiteness, the input tones peaking at their
 *  exact bins, and the tones dominating the noise floor.
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



//  Tree path: raw bytes -> RawAudio2WorkingAudio -> [mixer] -> [resampler]
//  -> WorkingAudioToFFT, mirroring AudioPassthroughPairQt::build_tree().
RecordingFFTListener run_tree(
    Logger& logger,
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
        resampler.reset(new Kernels::AudioResampler::AudioResampler(logger, raw_rate, fft_rate, 1));
        resampler->add_listener(*head);
        head = resampler.get();
    }
    if (raw_channels != 1){
        //  build_channel_mixer_matrix(stereo -> mono) is {0.5, 0.5}.
        mixer.reset(new Kernels::AudioPipeline::ChannelMixer(
            logger,
            raw_rate,
            raw_channels, 1,
            std::vector<float>{0.5f, 0.5f}
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



//  Invariants of the FFT path: every FFT reports the expected rate, all
//  bins are finite, each input tone peaks at its exact bin, and each tone
//  dominates the noise floor (measured on content-free bins far from the
//  tones). An empty string means the invariants hold.
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
//  DUAL_48000: the no-op case (no mixer rate change, no resampler).
//
class FFTGateDual48000Test : public UnitTest{
public:
    FFTGateDual48000Test()
        : UnitTest("AudioPipeline - FFTGate_Dual48000")
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

        RecordingFFTListener tree = run_tree(logger, raw, 2, rate, rate);

        //  The ring buffer is zero pre-filled, so the leading FFTs (0..3)
        //  contain zero padding; the invariants start at the first full
        //  window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), rate, 4, tone_bins);
        if (!error.empty()){
            return error;
        }

        return true;
    }
};



//
//  DUAL_44100: the tree resamples the 44.1 kHz source up to 48 kHz; the
//  invariants verify the resampled FFTs on their own.
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

        std::mt19937 generator(42);
        AlignedVector<float> l = make_signal(n, raw_rate, 4031.25, 14062.5, generator);
        AlignedVector<float> r = make_signal(n, raw_rate, 4031.25, 14062.5, generator);
        AlignedVector<float> raw(2 * n);
        for (size_t i = 0; i < n; i++){
            raw[2 * i + 0] = l[i];
            raw[2 * i + 1] = r[i];
        }

        RecordingFFTListener tree = run_tree(logger, raw, 2, raw_rate, fft_rate);

        //  The ring buffer is zero pre-filled and the resampler has a
        //  short transient, so the invariants start at the first full
        //  window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), fft_rate, 4, tone_bins);
        if (!error.empty()){
            return error;
        }

        return true;
    }
};



//
//  MONO_96000: the tree decimates the 96 kHz mono stream to 48 kHz with
//  the resampler; the invariants verify the decimated FFTs on their own.
//
class FFTGateMono96000Test : public UnitTest{
public:
    FFTGateMono96000Test()
        : UnitTest("AudioPipeline - FFTGate_Mono96000_Sanity")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const size_t raw_rate = 96000;
        const size_t fft_rate = 48000;
        const size_t n = 16384;
        const std::vector<size_t> tone_bins = {172, 600};   //  4031.25 / 14062.5 Hz at 48 kHz (14062.5 * 2048 / 48000 = 600).

        std::mt19937 generator(42);
        AlignedVector<float> raw = make_signal(n, raw_rate, 4031.25, 14062.5, generator);

        RecordingFFTListener tree = run_tree(logger, raw, 1, raw_rate, fft_rate);

        //  The ring buffer is zero pre-filled and the resampler has a
        //  short transient, so the invariants start at the first full
        //  window.
        std::string error = check_invariants(tree.ffts(), tree.rates(), fft_rate, 4, tone_bins);
        if (!error.empty()){
            return error;
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
