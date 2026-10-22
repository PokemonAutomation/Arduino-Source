/*  Audio Resampler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioResampler_H
#define PokemonAutomation_Kernels_AudioResampler_H

#include <cstddef>
#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "Kernels_AudioResampler_BlockKernels.h"
#include "Kernels_AudioResampler_WeightTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



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
class AudioResampler : public AudioPipeline::WorkingAudioNode{
public:
    ~AudioResampler();
    AudioResampler(
        Logger& logger,
        size_t in_sample_rate,
        size_t out_sample_rate,
        size_t channels,
        size_t buffer_size = 4096 + 256, //  4k + break super-alignment
        uint32_t taps = 32,
        double beta = 9.0
    );

    uint32_t taps() const{
        return m_taps;
    }
    size_t block_size() const{
        return m_block.sample_block;
    }

    virtual void add_listener(AudioPipeline::WorkingAudioStreamListener& listener) override;
    virtual void remove_listener(AudioPipeline::WorkingAudioStreamListener& listener) override;

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    //  Input samples a block starting at wrapped index wr needs, including
    //  the kernel's reads past the last window. The kernel's implied input
    //  overshoot is sample_block - 1 samples past the last window's start.
    size_t input_samples_needed(size_t wr) const;

    //  Emit one block over per-channel input bases and advance the
    //  per-push stream progress (out / wr / periods / cursor / out_off).
    //  in_base(c) is the input base pointer of channel c.
    template <typename InBase>
    size_t emit_block(
        size_t in_index, InBase in_base,
        size_t& out, size_t& wr, size_t& periods, size_t& cursor, size_t& out_off
    );

    void flush(size_t count);


private:
    const uint32_t m_taps;

    //  The dispatched block kernel.
    const BlockKernel m_block;

    //  The weight table, built from the block's sizes; it extends its
    //  allocations for the kernel's implied overshoots.
    WeightTable m_table;

    //  Input samples per extended period (m_table.period_in() times the
    //  period extension).
    const size_t m_period_in_extended;

    //  Extended output period: the smallest multiple of the table's period
    //  that is >= sample_block, so a block wraps it at most once and
    //  advancing the wrapped index is a compare-and-subtract, not a
    //  modulo.
    const size_t m_period_out_extended;

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

    //  Stream state.
    size_t m_in_index = 0;   //  Absolute index of the next input to be pushed.
    size_t m_out_index = 0;  //  Absolute index of the next output to produce.
    size_t m_wr = 0;         //  m_out_index within the extended output period.
    size_t m_periods = 0;    //  Extended output periods m_out_index spans.
    size_t m_carry_avail = 0;  //  Unprocessed input samples in the carries.
};



}
}
}
#endif
