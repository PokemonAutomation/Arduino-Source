/*  Audio Resampler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampler_H
#define PokemonAutomation_Kernels_AudioResampler_H

#include <cstddef>
#include <functional>
#include <vector>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



//  Per-ISA block kernel runners. Each one computes one block of consecutive
//  output samples from the weight table:
//
//      out_samples[c] = sum over taps t of
//          table row t, column out_index + c
//          x in_samples[in_sample_index[out_index + c] + t - in_index]
//
//  `in_index` is the absolute input stream index of in_samples[0], and
//  `out_index` is the absolute output stream index wrapped to the table
//  period. The runners are defined in per-ISA translation units and are
//  only referenced when the corresponding dispatching is enabled.
void run_block32_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);
void run_block128_up_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);
void run_block128_down2_x64_AVX512(
    const WeightTable& table,
    size_t in_index, const float* in_samples,
    size_t out_index, float* out_samples
);



//
//  A streaming node that resamples a working-format (float, channel-strided)
//  audio stream from one sample rate to another using the Kaiser-window
//  sinc kernel from the weight table.
//
//  Input samples are pushed with push_samples(); the resampled output is
//  delivered to the listeners in chunks of at most the output buffer size.
//  All channels are operated on identically; the kernels read directly from
//  the input buffer and write into the internal output buffer.
//
class AudioResampler : public AudioPipeline::WorkingAudioStreamListener{
public:
    ~AudioResampler();
    AudioResampler(
        size_t in_sample_rate,
        size_t out_sample_rate,
        size_t channels,
        size_t buffer_size = 4096 + 256, //  4k + break super-alignment
        uint32_t taps = 16,
        double beta = 9.0
    );

    size_t out_sample_rate() const{
        return m_out_rate;
    }
    uint32_t taps() const{
        return m_taps;
    }
    size_t block_size() const{
        return m_kinfo.block_size;
    }

    void add_listener(AudioPipeline::WorkingAudioStreamListener& listener);
    void remove_listener(AudioPipeline::WorkingAudioStreamListener& listener);

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    //  Kernel dispatch.
    enum class KernelKind{
        Default,
        SSE2,
        AVX512_POC,
        AVX512_Up,
        AVX512_Down2,
    };
    struct KernelInfo{
        KernelKind kind;
        size_t block_size;
        //  The kernel's input reads extend to `index[wr + need_last] +
        //  taps + need_os` samples past the block's first window start.
        size_t need_last;
        size_t need_os;
        //  Minimum number of weight table rows the kernel reads.
        size_t min_taps;
    };
    static KernelInfo kernel_info(
        size_t in_sample_rate, size_t out_sample_rate, uint32_t taps
    );

    void flush(size_t count);


private:
    const size_t m_out_rate;
    const uint32_t m_taps;
    const KernelInfo m_kinfo;
    //  Input samples per output period (in_sample_rate / gcd).
    const size_t m_period_in;
    WeightTable m_table;
    //  Floats per channel in m_buffer. At least the requested buffer size,
    //  rounded up so a block never straddles the channel boundary.
    const size_t m_buffer_stride;
    AlignedVector<float> m_buffer;
    size_t m_carry_capacity = 0;
    //  Per-channel buffers for the input samples that don't line up with a
    //  kernel block. The carry always starts at the next window's start.
    std::vector<AlignedVector<float>> m_carry;
    //  The largest multiple of the block size that fits in the buffer.
    const size_t m_flush_at;

    ListenerSet<AudioPipeline::WorkingAudioStreamListener> m_listeners;

    std::function<void(
        const WeightTable&, size_t, const float*, size_t, float*
    )> m_kernel;

    //  Stream state.
    size_t m_out_count = 0;  //  Absolute index of the next output to produce.
    size_t m_in_cursor = 0;  //  Absolute index of the next input to be pushed.
    size_t m_carry_avail = 0;  //  Unprocessed input samples in the carries.
};



}
}
}
#endif
