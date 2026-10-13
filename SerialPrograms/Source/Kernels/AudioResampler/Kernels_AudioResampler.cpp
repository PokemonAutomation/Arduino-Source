/*  Audio Resampler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cstring>
#include <numeric>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_AudioResampler_BlockKernel_Default.h"
#ifdef PA_AutoDispatch_x64_08_Nehalem
#include "Kernels_AudioResampler_BlockKernel_x64_SSE2.h"
#endif
#include "Kernels_AudioResampler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



AudioResampler::~AudioResampler() = default;
AudioResampler::KernelInfo AudioResampler::kernel_info(
    size_t in_sample_rate, size_t out_sample_rate, uint32_t taps
){
    KernelInfo info;
    info.kind = KernelKind::Default;
    info.block_size = 8;
    info.need_last = 7;
    info.need_os = 0;
    info.min_taps = taps;

#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        //  The permute-based kernels handle any tap count. The index delta
        //  within a 16-output group bounds how many input lanes each load
        //  must span: 15 for upsampling (in <= out) and 31 for downsampling
        //  up to 2x. Stronger downsampling falls back to the transpose POC.
        if (in_sample_rate <= out_sample_rate){
            info.kind = KernelKind::AVX512_Up;
            info.block_size = 128;
            info.need_last = 112;
            info.need_os = 15;
            info.min_taps = taps;
        } else if (in_sample_rate <= 2 * out_sample_rate){
            info.kind = KernelKind::AVX512_Down2;
            info.block_size = 128;
            info.need_last = 112;
            info.need_os = 31;
            info.min_taps = taps;
        } else {
            info.kind = KernelKind::AVX512_POC;
            info.block_size = 32;
            info.need_last = 31;
            size_t padded = ((size_t)taps + 7) / 8 * 8;
            info.need_os = padded - taps;
            info.min_taps = padded;
        }
        return info;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        //  The FMA3 block16_tap4 kernel is deliberately not used: its 8x4
        //  transpose routes the upper half of each output group onto the
        //  wrong taps. The SSE2 tap2 kernel is correct for any tap count
        //  (the weight table pads the odd tap row with zeros).
        info.kind = KernelKind::SSE2;
        info.block_size = 8;
        info.need_last = 7;
        info.need_os = taps & 1;
        info.min_taps = taps + (taps & 1);
        return info;
    }
#endif
    return info;
}
AudioResampler::AudioResampler(
    size_t in_sample_rate,
    size_t out_sample_rate,
    size_t channels,
    size_t buffer_size,
    uint32_t taps,
    double beta
)
    : AudioPipeline::WorkingAudioStreamListener(in_sample_rate, channels)
    , m_out_rate(out_sample_rate)
    , m_taps(taps)
    , m_kinfo(kernel_info(in_sample_rate, out_sample_rate, taps))
    , m_period_in(in_sample_rate / std::gcd(in_sample_rate, out_sample_rate))
    , m_table(
        in_sample_rate, out_sample_rate, taps, beta,
        out_sample_rate / std::gcd(in_sample_rate, out_sample_rate) + m_kinfo.block_size,
        m_kinfo.min_taps
    )
    , m_buffer_stride(std::max(buffer_size, m_kinfo.block_size))
    , m_buffer(channels * m_buffer_stride)
    , m_flush_at((buffer_size / m_kinfo.block_size) * m_kinfo.block_size)
{
    if (in_sample_rate == 0 || out_sample_rate == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid sample rate.");
    }
    if (channels == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid channel count.");
    }
    if (buffer_size == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid buffer size.");
    }
    if (taps == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid tap count.");
    }

    switch (m_kinfo.kind){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case KernelKind::SSE2:
        m_kernel = &run_block8_tap2_x64_SSE2;
        break;
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    case KernelKind::AVX512_POC:
        m_kernel = &run_block32_x64_AVX512;
        break;
    case KernelKind::AVX512_Up:
        m_kernel = &run_block128_up_x64_AVX512;
        break;
    case KernelKind::AVX512_Down2:
        m_kernel = &run_block128_down2_x64_AVX512;
        break;
#endif
    case KernelKind::Default:
        m_kernel = &run_block8_Default;
        break;
    default:
        break;
    }

    //  Size the carry buffers. The carry never holds more than one block's
    //  worth of input (see push_samples), so the worst-case block read plus
    //  a little slack is enough.
    size_t max_need = 0;
    const uint32_t* index = m_table.in_sample_index();
    for (size_t wr = 0; wr < m_table.period(); wr++){
        size_t need = index[wr + m_kinfo.need_last] + m_taps + m_kinfo.need_os - index[wr];
        if (need > max_need){
            max_need = need;
        }
    }
    m_carry_capacity = max_need + 64;
    m_carry.reserve(channels);
    for (size_t c = 0; c < channels; c++){
        m_carry.emplace_back(m_carry_capacity);
    }
}

void AudioResampler::add_listener(AudioPipeline::WorkingAudioStreamListener& listener){
    if (m_out_rate != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (this->channels() != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void AudioResampler::remove_listener(AudioPipeline::WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}

void AudioResampler::flush(size_t count){
    if (count == 0){
        return;
    }
    m_listeners.run_method(
        &AudioPipeline::WorkingAudioStreamListener::push_samples,
        count, m_buffer_stride, m_buffer.data()
    );
}

void AudioResampler::push_samples(size_t samples, size_t stride, const float* data){
    if (samples == 0){
        return;
    }

    const uint32_t* index = m_table.in_sample_index();
    const uint32_t taps = m_table.taps();
    const size_t period = m_table.period();
    const size_t block = m_kinfo.block_size;
    const size_t channels = this->channels();

    //  Input samples a block starting at wrapped index wr needs, including
    //  the kernel's reads past the last window.
    auto need = [&](size_t wr)->size_t{
        return index[wr + m_kinfo.need_last] + taps + m_kinfo.need_os - index[wr];
    };

    size_t out = m_out_count;
    size_t wr = out % period;

    //  Absolute input position of the next output's window start.
    size_t cursor = index[wr] + (out / period) * m_period_in;
    const size_t pushed_end = m_in_cursor + samples;

    //  When downsampling by a lot the cursor jumps past the pushed data.
    //  Drop the leading samples that no window will read.
    size_t n = samples;
    if (cursor > m_in_cursor){
        size_t skip = std::min(n, cursor - m_in_cursor);
        data += skip;
        n -= skip;
        m_in_cursor += skip;
        if (n == 0){
            m_in_cursor = pushed_end;
            m_carry_avail = 0;
            return;
        }
    }

    //  `carry` is the count of input samples in the carry buffers, which
    //  hold [cursor, cursor + carry). When negative the data pointer leads
    //  the cursor by -carry samples. The data starts at absolute position S.
    size_t S = m_in_cursor;
    ptrdiff_t carry = (ptrdiff_t)S - (ptrdiff_t)cursor;

    //  Upper bound on the outputs this push can produce. With extreme
    //  upsampling the cursor barely advances between blocks, so this keeps
    //  the block loops from running away.
    const size_t available = (pushed_end > cursor) ? (pushed_end - cursor) : 0;
    const size_t out_limit = available * m_out_rate / this->sample_rate() + block;

    size_t out_off = 0;

    //  Emit one block over per-channel input bases and advance the stream.
    auto emit_block = [&](size_t in_index, auto in_base)->size_t{
        if (out_off + block > m_flush_at){
            flush(out_off);
            out_off = 0;
        }
        float* out_base = m_buffer.data() + out_off;
        for (size_t c = 0; c < channels; c++){
            m_kernel(m_table, in_index, in_base(c), wr, out_base + c * m_buffer_stride);
        }
        out_off += block;

        size_t consumed = index[wr + block] - index[wr];
        out += block;
        wr = (wr + block) % period;
        cursor += consumed;
        return consumed;
    };

    //  Run a block on the carry buffers.
    auto emit_carry_block = [&]()->size_t{
        size_t consumed = emit_block(index[wr], [&](size_t c){
            return m_carry[c].data();
        });
        if ((ptrdiff_t)consumed >= carry){
            carry = 0;
        } else {
            for (size_t c = 0; c < channels; c++){
                memmove(
                    m_carry[c].data(),
                    m_carry[c].data() + consumed,
                    (size_t)carry * sizeof(float)
                );
            }
            carry -= (ptrdiff_t)consumed;
        }
        return consumed;
    };

    while (out - m_out_count < out_limit){
        const size_t need_w = need(wr);

        //  Not enough input to complete the next block.
        if ((ptrdiff_t)carry + (ptrdiff_t)n < (ptrdiff_t)need_w){
            break;
        }

        if (carry >= 0 && (size_t)carry >= need_w){
            //  The carry alone holds the full window.
            emit_carry_block();
        } else if (carry <= 0){
            //  The window starts at or before the data start.
            const size_t in_index = S - (out / period) * m_period_in;
            const size_t consumed = emit_block(in_index, [&](size_t c){
                return data + c * stride;
            });
            data += consumed;
            S += consumed;
            n -= consumed;
        } else {
            //  The window straddles the carry and the data. Copy the rest of
            //  the window into the carry and run the block on it.
            const size_t k = need_w - (size_t)carry;
            for (size_t c = 0; c < channels; c++){
                memcpy(
                    m_carry[c].data() + (size_t)carry,
                    data + c * stride,
                    k * sizeof(float)
                );
            }
            data += k;
            S += k;
            n -= k;
            carry = (ptrdiff_t)need_w;
            emit_carry_block();
        }
    }

    //  Save the unprocessed input tail in the carry buffers.
    if (n > 0){
        if (carry < 0){
            const size_t skip = (size_t)(-carry);
            if (n <= skip){
                n = 0;
            } else {
                data += skip;
                n -= skip;
                carry = 0;
            }
        }
        if (n > 0){
            for (size_t c = 0; c < channels; c++){
                memcpy(
                    m_carry[c].data() + (size_t)carry,
                    data + c * stride,
                    n * sizeof(float)
                );
            }
            carry += (ptrdiff_t)n;
        }
    }

    //  Empty the output buffer.
    if (out_off > 0){
        flush(out_off);
    }

    m_out_count = out;
    m_in_cursor = pushed_end;
    m_carry_avail = (carry > 0) ? (size_t)carry : 0;
}



}
}
}
