/*  Audio Resampling
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <cmath>
#include <numeric>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AudioResampling.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



float dot_product_Default(const float* a, const float* b, size_t length);
float dot_product_x86_SSE41(const float* a, const float* b, size_t length);
float dot_product_x86_AVX2(const float* a, const float* b, size_t length);
float dot_product_arm64_NEON(const float* a, const float* b, size_t length);

float dot_product(const float* a, const float* b, size_t length){
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return dot_product_x86_AVX2(a, b, length);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return dot_product_x86_SSE41(a, b, length);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return dot_product_arm64_NEON(a, b, length);
    }
#endif
    return dot_product_Default(a, b, length);
}



void mix_audio_channels(
    float* out, size_t out_channels,
    const float* in, size_t in_channels,
    size_t frames
){
    if (out_channels == in_channels){
        memcpy(out, in, frames * in_channels * sizeof(float));
        return;
    }
    if (out_channels == 1){
        const float scale = 1.0f / (float)in_channels;
        for (size_t f = 0; f < frames; f++){
            float sum = 0;
            for (size_t c = 0; c < in_channels; c++){
                sum += in[c];
            }
            out[0] = sum * scale;
            in += in_channels;
            out += 1;
        }
        return;
    }
    for (size_t f = 0; f < frames; f++){
        for (size_t c = 0; c < out_channels; c++){
            if (in_channels == 1){
                out[c] = c < 2 ? in[0] : 0.0f;
            }else{
                out[c] = c < in_channels ? in[c] : 0.0f;
            }
        }
        in += in_channels;
        out += out_channels;
    }
}



namespace{

//  Kaiser window shape parameter. 8.0 gives about 80dB of stopband attenuation.
const double KAISER_BETA = 8.0;

//  Cutoff frequency as a fraction of the lower Nyquist frequency. Leaves room
//  for the filter's transition band so that it is (nearly) fully attenuated by
//  the Nyquist frequency. e.g. for 48000 -> 44100 the passband ends at ~19.8kHz.
const double CUTOFF_RATIO = 0.9;

//  Zeroth-order modified Bessel function of the first kind, used by the Kaiser
//  window. Computed with its power series, which converges quickly for the small
//  arguments used here.
double bessel_i0(double x){
    double sum = 1.0;
    double term = 1.0;
    double half_x = x / 2.0;
    for (int k = 1; k < 50; k++){
        term *= half_x / k;
        double t2 = term * term;
        sum += t2;
        if (t2 < sum * 1e-17){
            break;
        }
    }
    return sum;
}

}


bool AudioResampler::is_supported(size_t input_rate, size_t output_rate){
    if (input_rate == 0 || output_rate == 0){
        return false;
    }
    size_t g = std::gcd(input_rate, output_rate);
    return output_rate / g <= MAX_PHASES;
}

AudioResampler::AudioResampler(size_t channels, size_t input_rate, size_t output_rate)
    : m_channels(channels)
    , m_history(channels, std::vector<float>(TAPS - 1, 0.0f))
    , m_time(0)
{
    if (channels == 0 || !is_supported(input_rate, output_rate)){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unsupported resampling: " + std::to_string(channels) + " channel(s), " +
            std::to_string(input_rate) + "Hz -> " + std::to_string(output_rate) + "Hz"
        );
    }
    size_t g = std::gcd(input_rate, output_rate);
    m_up = output_rate / g;
    m_down = input_rate / g;

    //  Design the prototype low-pass filter at the upsampled rate. It has
    //  `L * TAPS` coefficients. The cutoff is in cycles per upsampled sample.
    const size_t length = m_up * TAPS;
    const double cutoff = 0.5 * CUTOFF_RATIO / (double)std::max(m_up, m_down);
    const double center = (double)(length - 1) / 2.0;
    const double window_scale = 1.0 / bessel_i0(KAISER_BETA);
    const double PI = 3.14159265358979323846;

    std::vector<double> prototype(length);
    for (size_t k = 0; k < length; k++){
        double x = (double)k - center;
        double sinc_arg = 2.0 * cutoff * x;
        double sinc = sinc_arg == 0 ? 1.0 : std::sin(PI * sinc_arg) / (PI * sinc_arg);
        double r = x / center;
        double window = bessel_i0(KAISER_BETA * std::sqrt(std::max(0.0, 1.0 - r * r))) * window_scale;
        //  Gain of L compensates for the L - 1 zeros inserted by upsampling.
        prototype[k] = 2.0 * cutoff * sinc * window * (double)m_up;
    }

    //  Split into phases. Output at upsampled time `t` with phase `p = t % L` and
    //  input index `i = t / L` is:
    //      sum over j in [0, TAPS) of prototype[p + j*L] * x[i - j]
    //  Store each phase reversed so this becomes a dot product with
    //  x[i - TAPS + 1 ... i], which is contiguous in the history buffer.
    m_coefficients.resize(length);
    for (size_t p = 0; p < m_up; p++){
        float* phase = m_coefficients.data() + p * TAPS;
        for (size_t j = 0; j < TAPS; j++){
            phase[TAPS - 1 - j] = (float)prototype[p + j * m_up];
        }
    }
}

void AudioResampler::process(std::vector<float>& out, const float* in, size_t frames){
    if (frames == 0){
        return;
    }

    //  Deinterleave the new frames after the history of each channel.
    for (size_t c = 0; c < m_channels; c++){
        std::vector<float>& history = m_history[c];
        history.resize(TAPS - 1 + frames);
        float* dest = history.data() + TAPS - 1;
        for (size_t f = 0; f < frames; f++){
            dest[f] = in[f * m_channels + c];
        }
    }

    //  Produce every output sample whose newest input sample is in this block.
    const uint64_t end = (uint64_t)frames * m_up;
    while (m_time < end){
        size_t i = (size_t)(m_time / m_up);
        size_t p = (size_t)(m_time % m_up);
        const float* phase = m_coefficients.data() + p * TAPS;
        for (size_t c = 0; c < m_channels; c++){
            //  x[i - TAPS + 1] is at index i in the history buffer.
            out.push_back(dot_product(phase, m_history[c].data() + i, TAPS));
        }
        m_time += m_down;
    }
    m_time -= end;

    //  Keep the last TAPS - 1 input samples for the next block.
    for (std::vector<float>& history : m_history){
        memmove(history.data(), history.data() + frames, (TAPS - 1) * sizeof(float));
        history.resize(TAPS - 1);
    }
}



bool AudioFormatConverter::is_supported(size_t input_rate, size_t output_rate){
    return (input_rate == output_rate && input_rate != 0) ||
        AudioResampler::is_supported(input_rate, output_rate);
}

AudioFormatConverter::AudioFormatConverter(
    size_t input_channels, size_t input_rate,
    size_t output_channels, size_t output_rate
)
    : m_input_channels(input_channels)
    , m_output_channels(output_channels)
{
    if (input_channels == 0 || output_channels == 0 || !is_supported(input_rate, output_rate)){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unsupported audio format conversion: " +
            std::to_string(input_channels) + " channel(s) at " + std::to_string(input_rate) + "Hz -> " +
            std::to_string(output_channels) + " channel(s) at " + std::to_string(output_rate) + "Hz"
        );
    }
    if (input_rate != output_rate){
        m_resampler = std::make_unique<AudioResampler>(
            std::min(input_channels, output_channels),
            input_rate, output_rate
        );
    }
}

const float* AudioFormatConverter::convert(const float* in, size_t frames, size_t& output_frames){
    const float* current = in;
    size_t channels = m_input_channels;

    //  Reduce the channel count before resampling.
    if (m_output_channels < channels){
        m_buffer0.resize(frames * m_output_channels);
        mix_audio_channels(m_buffer0.data(), m_output_channels, current, channels, frames);
        current = m_buffer0.data();
        channels = m_output_channels;
    }

    if (m_resampler){
        m_buffer1.clear();
        m_resampler->process(m_buffer1, current, frames);
        current = m_buffer1.data();
        frames = m_buffer1.size() / channels;
    }

    //  Increase the channel count after resampling.
    if (m_output_channels > channels){
        m_buffer0.resize(frames * m_output_channels);
        mix_audio_channels(m_buffer0.data(), m_output_channels, current, channels, frames);
        current = m_buffer0.data();
    }

    output_frames = frames;
    return current;
}



}
}
}
