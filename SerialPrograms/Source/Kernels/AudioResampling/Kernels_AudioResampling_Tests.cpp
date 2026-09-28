/*  Audio Resampling Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioResampling.h"
#include "Kernels_AudioResampling_Tests.h"

namespace PokemonAutomation{
namespace Kernels{

namespace AudioResampling{
    float dot_product_Default(const float* a, const float* b, size_t length);
}
using namespace AudioResampling;


namespace{

const double PI = 3.14159265358979323846;


// Check the run-time dispatched `dot_product()` (SSE4.1 / AVX2 / NEON on
// supported CPUs) against the scalar version for every length up to 100,
// which covers all the vector-width remainder paths.
std::string test_dot_product(){
    std::mt19937 rng(0);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::vector<float> a(100), b(100);
    for (size_t c = 0; c < 100; c++){
        a[c] = dist(rng);
        b[c] = dist(rng);
    }
    for (size_t length = 0; length <= 100; length++){
        float expected = dot_product_Default(a.data(), b.data(), length);
        float actual = dot_product(a.data(), b.data(), length);
        if (std::fabs(expected - actual) > 1e-5f * (1.0f + (float)length)){
            return "dot_product() mismatch at length " + std::to_string(length) + ": " +
                std::to_string(actual) + " vs " + std::to_string(expected);
        }
    }
    return "";
}


// Check each rule of `mix_audio_channels()` on two frames.
std::string test_mix_channels(){
    struct Case{
        size_t in_channels;
        size_t out_channels;
        std::vector<float> in;
        std::vector<float> expected;
    };
    const Case CASES[] = {
        {2, 2, {0.1f, 0.2f, 0.3f, 0.4f}, {0.1f, 0.2f, 0.3f, 0.4f}},
        {2, 1, {0.2f, 0.4f, -0.6f, 0.2f}, {0.3f, -0.2f}},
        {1, 2, {0.5f, -0.25f}, {0.5f, 0.5f, -0.25f, -0.25f}},
        {1, 4, {0.5f, -0.25f}, {0.5f, 0.5f, 0, 0, -0.25f, -0.25f, 0, 0}},
        {2, 3, {0.1f, 0.2f, 0.3f, 0.4f}, {0.1f, 0.2f, 0, 0.3f, 0.4f, 0}},
        {3, 2, {0.1f, 0.2f, 0.9f, 0.3f, 0.4f, 0.9f}, {0.1f, 0.2f, 0.3f, 0.4f}},
    };
    for (const Case& test : CASES){
        std::vector<float> out(test.expected.size(), 123.0f);
        mix_audio_channels(out.data(), test.out_channels, test.in.data(), test.in_channels, 2);
        for (size_t c = 0; c < out.size(); c++){
            if (std::fabs(out[c] - test.expected[c]) > 1e-6f){
                return "mix_audio_channels() " + std::to_string(test.in_channels) + " -> " +
                    std::to_string(test.out_channels) + " channels: wrong sample at index " + std::to_string(c);
            }
        }
    }
    return "";
}


// Resample one second of a stereo sine wave (left = sine, right = -sine) and check:
// 1. The number of output frames is exactly the number of output sample times
//    that fall within the input.
// 2. After the filter's warm-up, every output sample matches the ideal sine at
//    that time (shifted by the filter's fixed delay). Passband ripple, leftover
//    upsampling images and aliasing all show up as error here.
// 3. Feeding the same input in random block sizes gives bit-identical output,
//    i.e. no clicks or drift at block boundaries.
std::string test_resample_sine(size_t input_rate, size_t output_rate, double frequency){
    const size_t CHANNELS = 2;
    const size_t frames = input_rate;
    const double amplitude = 0.5;
    std::vector<float> in(frames * CHANNELS);
    for (size_t f = 0; f < frames; f++){
        float s = (float)(amplitude * std::sin(2 * PI * frequency * (double)f / (double)input_rate));
        in[f * CHANNELS + 0] = s;
        in[f * CHANNELS + 1] = -s;
    }

    std::vector<float> whole;
    AudioResampler(CHANNELS, input_rate, output_rate).process(whole, in.data(), frames);

    const std::string name = std::to_string(input_rate) + " -> " + std::to_string(output_rate);

    size_t g = std::gcd(input_rate, output_rate);
    uint64_t up = output_rate / g;
    uint64_t down = input_rate / g;
    uint64_t expected_frames = (frames * up + down - 1) / down;
    if (whole.size() != expected_frames * CHANNELS){
        return name + ": expected " + std::to_string(expected_frames) + " frames, got " +
            std::to_string(whole.size() / CHANNELS);
    }

    //  Output frame n is at upsampled time n * M. The filter delays it by
    //  (L * TAPS - 1) / 2 upsampled units. Skip the warm-up: the output frames
    //  whose filter window still reaches before the first input sample.
    const double delay = (double)(up * AudioResampler::TAPS - 1) / 2.0;
    const size_t warm_up = (size_t)((up * AudioResampler::TAPS + down - 1) / down);
    double max_error = 0;
    for (size_t n = warm_up; n < expected_frames; n++){
        double t = ((double)n * (double)down - delay) / (double)(up * input_rate);
        double expected = amplitude * std::sin(2 * PI * frequency * t);
        max_error = std::max(max_error, std::fabs(whole[n * CHANNELS + 0] - expected));
        max_error = std::max(max_error, std::fabs(whole[n * CHANNELS + 1] + expected));
    }
    if (max_error > 2e-3){
        return name + ": max error against ideal sine is " + std::to_string(max_error);
    }

    std::vector<float> chunked;
    AudioResampler resampler(CHANNELS, input_rate, output_rate);
    std::mt19937 rng(1);
    std::uniform_int_distribution<size_t> block_size(0, 1000);
    for (size_t f = 0; f < frames;){
        size_t block = std::min(block_size(rng), frames - f);
        resampler.process(chunked, in.data() + f * CHANNELS, block);
        f += block;
    }
    if (chunked != whole){
        return name + ": output depends on how the input is split into blocks.";
    }
    return "";
}


// Resample a tone that is above the output's Nyquist frequency. It cannot be
// represented at the output rate, so a correct resampler removes it. Without
// the low-pass filter it would alias into the audible band at full volume.
std::string test_anti_alias(size_t input_rate, size_t output_rate, double frequency){
    const size_t frames = input_rate;
    std::vector<float> in(frames);
    for (size_t f = 0; f < frames; f++){
        in[f] = (float)(0.5 * std::sin(2 * PI * frequency * (double)f / (double)input_rate));
    }
    std::vector<float> out;
    AudioResampler(1, input_rate, output_rate).process(out, in.data(), frames);

    double sum_sqr = 0;
    size_t count = 0;
    for (size_t n = AudioResampler::TAPS; n < out.size(); n++){
        sum_sqr += (double)out[n] * out[n];
        count++;
    }
    double rms = std::sqrt(sum_sqr / (double)count);
    double input_rms = 0.5 / std::sqrt(2.0);
    double attenuation_db = 20 * std::log10(rms / input_rms);
    if (attenuation_db > -60){
        return std::to_string(input_rate) + " -> " + std::to_string(output_rate) + ": " +
            std::to_string(frequency) + "Hz is only attenuated by " + std::to_string(-attenuation_db) + "dB";
    }
    return "";
}


// Check `AudioFormatConverter` in both channel directions: mixing before
// resampling (stereo 48000 -> mono 16000) and after (mono 96000 -> stereo 48000).
std::string test_format_converter(){
    const double frequency = 1000;
    struct Case{
        size_t in_channels, in_rate, out_channels, out_rate;
    };
    const Case CASES[] = {
        {2, 48000, 1, 16000},
        {1, 96000, 2, 48000},
        {2, 48000, 1, 48000},
    };
    for (const Case& test : CASES){
        const std::string name =
            std::to_string(test.in_channels) + "ch " + std::to_string(test.in_rate) + " -> " +
            std::to_string(test.out_channels) + "ch " + std::to_string(test.out_rate);

        std::vector<float> in(test.in_rate * test.in_channels);
        for (size_t f = 0; f < test.in_rate; f++){
            float s = (float)(0.5 * std::sin(2 * PI * frequency * (double)f / (double)test.in_rate));
            for (size_t c = 0; c < test.in_channels; c++){
                in[f * test.in_channels + c] = s;
            }
        }
        AudioFormatConverter converter(test.in_channels, test.in_rate, test.out_channels, test.out_rate);
        size_t out_frames;
        const float* out = converter.convert(in.data(), test.in_rate, out_frames);

        size_t expected_frames = test.out_rate;
        if (out_frames < expected_frames - 1 || out_frames > expected_frames + 1){
            return name + ": expected about " + std::to_string(expected_frames) +
                " frames, got " + std::to_string(out_frames);
        }
        double peak = 0;
        for (size_t n = AudioResampler::TAPS; n < out_frames; n++){
            for (size_t c = 0; c < test.out_channels; c++){
                peak = std::max(peak, (double)std::fabs(out[n * test.out_channels + c]));
            }
            if (test.out_channels == 2 && out[n * 2 + 0] != out[n * 2 + 1]){
                return name + ": left and right differ at frame " + std::to_string(n);
            }
        }
        if (std::fabs(peak - 0.5) > 2e-3){
            return name + ": expected peak amplitude 0.5, got " + std::to_string(peak);
        }
    }
    return "";
}


template <typename Function>
class Test_AudioResampling : public UnitTest{
public:
    Test_AudioResampling(const std::string& name, Function function)
        : UnitTest("Kernels::AudioResampling - " + name)
        , m_function(std::move(function))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        std::string error = m_function();
        if (error.empty()){
            return true;
        }
        return UnitTestResult(error);
    }

private:
    Function m_function;
};

template <typename Function>
void add_test(UnitTestDatabase& database, const std::string& name, Function function){
    database.add<Test_AudioResampling<Function>>(name, std::move(function));
}

}



void add_tests_AudioResampling(UnitTestDatabase& database){
    add_test(database, "DotProduct", test_dot_product);
    add_test(database, "MixChannels", test_mix_channels);
    const size_t RATES[][2] = {
        {48000, 44100},
        {44100, 48000},
        {48000, 16000},
        {16000, 48000},
        {96000, 48000},
        {32000, 44100},
    };
    for (const auto& rates : RATES){
        size_t input_rate = rates[0];
        size_t output_rate = rates[1];
        add_test(
            database,
            "Sine " + std::to_string(input_rate) + " -> " + std::to_string(output_rate),
            [=]{ return test_resample_sine(input_rate, output_rate, 1000); }
        );
    }
    add_test(database, "AntiAlias 48000 -> 16000", []{ return test_anti_alias(48000, 16000, 12000); });
    add_test(database, "AntiAlias 48000 -> 44100", []{ return test_anti_alias(48000, 44100, 23000); });
    add_test(database, "FormatConverter", test_format_converter);
}



}
}
