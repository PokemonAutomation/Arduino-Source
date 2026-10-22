/*  Audio Resampler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cstring>
#include <numeric>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_AudioResampler_BlockKernels.h"
#include "Kernels_AudioResampler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



AudioResampler::~AudioResampler() = default;
AudioResampler::AudioResampler(
    Logger& logger,
    size_t in_sample_rate,
    size_t out_sample_rate,
    size_t channels,
    size_t buffer_size,
    uint32_t taps,
    double beta
)
    : AudioPipeline::WorkingAudioNode(in_sample_rate, out_sample_rate, channels, channels)
    , m_taps(taps)
    , m_block(get_block_kernel(in_sample_rate, out_sample_rate))
    , m_table(
        in_sample_rate, out_sample_rate, taps, beta,
        m_block.sample_block, m_block.tap_block
    )
    , m_period_in_extended(m_table.period_in() * (m_table.period_out_extended() / m_table.period_out()))
    , m_period_out_extended(m_table.period_out_extended())
    , m_buffer_stride(std::max(buffer_size, m_block.sample_block))
    , m_buffer(channels * m_buffer_stride)
    , m_flush_at((buffer_size / m_block.sample_block) * m_block.sample_block)
{
    logger.log(
        "Initializing AudioResampler: " + tostr_u_commas(in_sample_rate) +
        " -> " + tostr_u_commas(out_sample_rate) +
        ", channels = " + tostr_u_commas(channels),
        COLOR_PURPLE
    );

    if (in_sample_rate == 0 || out_sample_rate == 0){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Invalid sample rate.");
    }
    if (channels == 0){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Invalid channel count.");
    }
    if (buffer_size == 0){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Invalid buffer size.");
    }
    if (taps == 0){
        throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Invalid tap count.");
    }


    //  Size the carry buffers. push_samples() never stages more than
    //  input_samples_needed(wr) input samples for the block at wrapped index wr
    //  - both the loop gate and the tail save bound against it - and need()
    //  already includes the kernel's implied read overshoots, so the max over
    //  all positions is an exact worst case and needs no slack.
    size_t max_input_samples_needed = 0;
    for (size_t wr = 0; wr < m_table.period_out(); wr++){
        max_input_samples_needed = std::max(max_input_samples_needed, input_samples_needed(wr));
    }
    m_carry_capacity = max_input_samples_needed;
    m_carry.reserve(channels);
    for (size_t c = 0; c < channels; c++){
        m_carry.emplace_back(m_carry_capacity);
    }
}

void AudioResampler::add_listener(AudioPipeline::WorkingAudioStreamListener& listener){
    if (out_sample_rate() != listener.sample_rate()){
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

size_t AudioResampler::input_samples_needed(size_t wr) const{
    const uint32_t* index = m_table.in_sample_index();
    const size_t block = m_block.sample_block;
    //  taps_allocated() bounds the kernel's input reads: the unrolls'
    //  final partial group reads at most
    //  ceil(taps / tap_block) * tap_block taps, and the table's padding to
    //  taps + tap_block - 1 rows is at least that.
    return index[wr + block - 1] + m_table.taps_allocated() + (block - 1) - index[wr];
}

template <typename InBase>
size_t AudioResampler::emit_block(
    size_t in_index, InBase in_base,
    size_t& out, size_t& wr, size_t& periods, size_t& cursor, size_t& out_off
){
    const uint32_t* index = m_table.in_sample_index();
    const size_t block = m_block.sample_block;

    if (out_off + block > m_flush_at){
        flush(out_off);
        out_off = 0;
    }
    float* out_base = m_buffer.data() + out_off;
    for (size_t c = 0; c < this->channels(); c++){
        m_block.fp(m_table, in_index, in_base(c), wr, out_base + c * m_buffer_stride);
    }
    out_off += block;

    size_t consumed = index[wr + block] - index[wr];
    out += block;
    wr += block;
    //  block <= m_period_out_extended, so this wraps at most once.
    if (wr >= m_period_out_extended){
        wr -= m_period_out_extended;
        periods += 1;
    }
    cursor += consumed;
    return consumed;
}

void AudioResampler::push_samples(size_t samples, size_t stride, const float* data){
    if (samples == 0){
        return;
    }

    const uint32_t* index = m_table.in_sample_index();
    const size_t block = m_block.sample_block;
    const size_t channels = this->channels();

    //  wr and periods decompose m_out_index over the extended output
    //  period. They are carried across calls and only the block loop
    //  below mutates them, so the push is division- and modulo-free
    //  throughout.
    size_t out_index = m_out_index;
    size_t wr = m_wr;
    size_t periods = m_periods;

    //  Absolute input position of the next output's window start.
    size_t cursor = index[wr] + periods * m_period_in_extended;
    const size_t pushed_end = m_in_index + samples;

    //  When downsampling by a lot the cursor jumps past the pushed data.
    //  Drop the leading samples that no window will read.
    size_t n = samples;
    if (cursor > m_in_index){
        size_t skip = std::min(n, cursor - m_in_index);
        data += skip;
        n -= skip;
        m_in_index += skip;
        if (n == 0){
            m_in_index = pushed_end;
            m_carry_avail = 0;
            return;
        }
    }

    //  `carry` is the count of input samples in the carry buffers, which
    //  hold [cursor, cursor + carry). When negative the data pointer leads
    //  the cursor by -carry samples. The data starts at absolute position
    //  in_index.
    size_t in_index = m_in_index;
    ptrdiff_t carry = (ptrdiff_t)in_index - (ptrdiff_t)cursor;

    //  Upper bound on the outputs this push can produce. With extreme
    //  upsampling the cursor barely advances between blocks, so this keeps
    //  the block loops from running away.
    const size_t available = (pushed_end > cursor) ? (pushed_end - cursor) : 0;
    const size_t out_limit = available * out_sample_rate() / this->sample_rate() + block;

    size_t out_off = 0;

    //  Run a block on the carry buffers.
    auto emit_carry_block = [&]()->size_t{
        size_t consumed = emit_block(
            index[wr],
            [&](size_t c){
                return m_carry[c].data();
            },
            out_index, wr, periods, cursor, out_off
        );
        if ((ptrdiff_t)consumed < carry){
            for (size_t c = 0; c < channels; c++){
                memmove(
                    m_carry[c].data(),
                    m_carry[c].data() + consumed,
                    (size_t)(carry - consumed) * sizeof(float)
                );
            }
        }
        return consumed;
    };

    while (out_index - m_out_index < out_limit){
        const size_t need_w = input_samples_needed(wr);

        //  Not enough input to complete the next block.
        if ((ptrdiff_t)carry + (ptrdiff_t)n < (ptrdiff_t)need_w){
            break;
        }

        if (carry >= 0 && (size_t)carry >= need_w){
            //  The carry alone holds the full window.
            emit_carry_block();
        }else if (carry <= 0){
            //  The window starts at or before the data start. The block's
            //  window-start span can exceed the remaining data (strong
            //  downsampling), so only advance as far as the data allows;
            //  the reconciliation below drops whatever is now in the past.
            //  Rewind the data base to the extended-period origin.
            const size_t rewind = in_index - periods * m_period_in_extended;
            const size_t consumed = emit_block(
                rewind,
                [&](size_t c){
                    return data + c * stride;
                },
                out_index, wr, periods, cursor, out_off
            );
            const size_t advance = std::min(consumed, n);
            data += advance;
            in_index += advance;
            n -= advance;
        }else{
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
            in_index += k;
            n -= k;
            carry = (ptrdiff_t)need_w;
            emit_carry_block();
        }

        //  The cursor advanced by the block's window-start span, which can
        //  exceed the input that was staged for the block. Drop any data
        //  samples that now sit before the cursor, and re-derive how far
        //  the data leads or lags the cursor (the carry buffers only hold
        //  [cursor, cursor + carry) when carry > 0).
        if (in_index < cursor){
            const size_t skip = std::min(n, cursor - in_index);
            data += skip;
            in_index += skip;
            n -= skip;
        }
        carry = (ptrdiff_t)in_index - (ptrdiff_t)cursor;
    }

    //  Save the unprocessed input tail in the carry buffers.
    if (n > 0){
        if (carry < 0){
            const size_t skip = (size_t)(-carry);
            if (n <= skip){
                n = 0;
            }else{
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

    m_out_index = out_index;
    m_wr = wr;
    m_periods = periods;
    m_in_index = pushed_end;
    m_carry_avail = (carry > 0) ? (size_t)carry : 0;
}



}
}
}
