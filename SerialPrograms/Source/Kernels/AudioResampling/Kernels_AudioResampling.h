/*  Audio Resampling
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Convert a stream of interleaved float audio frames to a different
 *  channel count and/or sample rate. This is used when an audio output device
 *  (e.g. Bluetooth or USB headphones) cannot play the capture card's format
 *  directly. For example, a capture card gives 48000Hz stereo while Bluetooth
 *  headphones may only accept 44100Hz stereo (A2DP profile) or 16000Hz mono
 *  (hands-free profile).
 *
 *      Sample type conversion (int <-> float) is not done here. That is in
 *  "Kernels/AudioStreamConversion".
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampling_H
#define PokemonAutomation_Kernels_AudioResampling_H

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <vector>

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampling{



// Return the dot product of two float arrays of the same length.
// This is the inner loop of `AudioResampler` (one FIR filter tap set applied to
// one channel's input history) and is dispatched at run time to SSE4.1, AVX2 or
// ARM NEON when available.
float dot_product(const float* a, const float* b, size_t length);


// Remap `frames` interleaved frames from `in_channels` channels to `out_channels`
// channels. `in` and `out` must not overlap.
//  - Same channel count: copy.
//  - Mono output: average all input channels. e.g. stereo (L, R) -> (L + R) / 2.
//  - Mono input: copy it into the first two output channels (left and right).
//  - Otherwise: copy each input channel to the output channel with the same index.
// Any output channels left over (e.g. surround channels) are filled with silence.
void mix_audio_channels(
    float* out, size_t out_channels,
    const float* in, size_t in_channels,
    size_t frames
);



// Streaming sample rate converter for interleaved float audio.
//
// It uses a polyphase windowed-sinc FIR filter, the standard method for
// rational-ratio resampling:
// 1. Reduce the ratio `output_rate / input_rate` to lowest terms `L / M`,
//    e.g. 48000 -> 44100 becomes 147 / 160.
// 2. Conceptually, upsample by `L` (insert L - 1 zeros between input samples),
//    low-pass filter, then keep every `M`-th sample. The low-pass cutoff is
//    slightly below the lower of the two Nyquist frequencies, so that it both
//    removes the images created by upsampling and prevents aliasing when
//    downsampling (e.g. 48000 -> 16000 removes everything above ~7.2kHz instead
//    of folding it back into the audible band).
// 3. In practice, the zeros are never materialized. The filter is split into
//    `L` phases of `TAPS` coefficients each, and each output sample is a single
//    `dot_product()` between one phase and the last `TAPS` input samples.
//
// The filter adds a fixed latency of about `TAPS / 2` input samples (0.5ms at 48000Hz).
// State (input history and the phase position) is kept across `process()` calls,
// so the output does not depend on how the input is split into blocks.
class AudioResampler{
public:
    //  Number of input samples each output sample is computed from.
    static constexpr size_t TAPS = 48;

    //  The largest `L` (number of filter phases) we allow. Every pair of common
    //  sample rates (8000 to 192000Hz in both the 44100 and 48000 families) is
    //  well below this. It limits the coefficient table to TAPS * MAX_PHASES floats.
    static constexpr size_t MAX_PHASES = 1024;

    //  Return whether converting between these two rates is supported, i.e. both
    //  are non-zero and the reduced ratio has at most `MAX_PHASES` phases.
    static bool is_supported(size_t input_rate, size_t output_rate);

    //  Throws InternalProgramError if `is_supported()` is false or channels is zero.
    AudioResampler(size_t channels, size_t input_rate, size_t output_rate);

    //  Resample `frames` interleaved input frames and append the resulting
    //  interleaved output frames to `out`. Each call produces about
    //  `frames * output_rate / input_rate` frames.
    void process(std::vector<float>& out, const float* in, size_t frames);

private:
    size_t m_channels;
    size_t m_up;        //  L
    size_t m_down;      //  M

    //  `m_up` phases of `TAPS` coefficients. Each phase is stored in reverse
    //  tap order so it can be dot-producted directly with the input history.
    std::vector<float> m_coefficients;

    //  One buffer per channel: the last `TAPS - 1` input samples of the previous
    //  block, followed by the current block.
    std::vector<std::vector<float>> m_history;

    //  Position of the next output sample in upsampled time units (1 / (input_rate * L)
    //  seconds), relative to the first input sample of the current block.
    uint64_t m_time;
};



// Convert interleaved float audio from one channel count and sample rate to another
// by combining `mix_audio_channels()` and `AudioResampler`.
// When reducing the channel count, channels are mixed before resampling. When
// increasing it, channels are mixed after resampling. This way the resampler
// always runs on the smaller number of channels.
class AudioFormatConverter{
public:
    //  Return whether `AudioResampler` supports these rates (always true if they are equal).
    static bool is_supported(size_t input_rate, size_t output_rate);

    //  Throws InternalProgramError if `is_supported()` is false or a channel count is zero.
    AudioFormatConverter(
        size_t input_channels, size_t input_rate,
        size_t output_channels, size_t output_rate
    );

    //  Convert `frames` input frames. Return a pointer to the output frames and
    //  write the number of output frames to `output_frames`. The returned buffer
    //  is owned by this class and is valid until the next call.
    const float* convert(const float* in, size_t frames, size_t& output_frames);

private:
    size_t m_input_channels;
    size_t m_output_channels;
    std::unique_ptr<AudioResampler> m_resampler;    //  Null if the rates are equal.
    std::vector<float> m_buffer0;
    std::vector<float> m_buffer1;
};



}
}
}
#endif
