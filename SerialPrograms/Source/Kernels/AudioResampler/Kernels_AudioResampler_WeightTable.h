/*  Audio Resampler - Weight Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioSampler_WeightTable_H
#define PokemonAutomation_Kernels_AudioSampler_WeightTable_H

#include <stdint.h>
#include "Common/Cpp/Containers/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{


class WeightTable{
public:
    WeightTable(
        size_t in_sample_rate,
        size_t out_sample_rate,
        uint32_t taps,
        double beta,

        //  The block kernel's input block size in output samples. The
        //  index/weight allocation is extended so a block starting at any
        //  position of the extended period stays in bounds.
        size_t sample_block,

        //  The block kernel's tap group size. The tap allocation is
        //  extended so the kernel's final partial tap group stays in
        //  bounds.
        size_t tap_block
    );

    //  The sample rates the table was built for.
    size_t in_sample_rate() const{
        return m_in_sample_rate;
    }

    size_t out_sample_rate() const{
        return m_out_sample_rate;
    }

    size_t taps() const{
        return m_taps;
    }

    //  The block sizes the table was built for. A table built for larger
    //  blocks is valid for smaller ones (the block sizes only extend the
    //  allocations, not the weights).
    size_t sample_block() const{
        return m_sample_block;
    }
    size_t tap_block() const{
        return m_tap_block;
    }

    //  Input samples per output period (in_sample_rate / gcd).
    size_t period_in() const{
        return m_period_in;
    }

    //  Output samples per period (out_sample_rate / gcd).
    size_t period_out() const{
        return m_period_out;
    }

    //  The extended output period: the smallest multiple of period_out()
    //  that is >= the sample block, so a block wraps it at most once.
    size_t period_out_extended() const{
        return m_period_out_extended;
    }

    //  time(output[0]) - time(input[0]), in seconds. Output sample c
    //  reconstructs the input at c * in_rate / out_rate + (taps - 1) / 2
    //  input samples: the window's time alignment sits at the median tap
    //  (the midpoint between the two middle taps for even tap counts),
    //  which is (taps - 1) / 2 input samples past the 1st tap, anchored
    //  at the ideal output time. Positive: the output's time 0 is this
    //  far into the input stream.
    double time_shift() const{
        return ((double)m_taps - 1.0) * 0.5 / (double)m_in_sample_rate;
    }

    //  Tap rows in the table (the requested taps, zero-padded for the
    //  kernel's tap group).
    size_t taps_allocated() const{
        return m_taps_allocated;
    }

    //  Index entries (and weight columns) in the table.
    size_t samples_allocated() const{
        return m_samples_allocated;
    }

    const uint32_t* in_sample_index() const{
        return m_index.data();
    }

    const float* tap_start(size_t tap_index) const{
        return m_data.data() + tap_index * m_tap_stride;
    }

    size_t tap_stride() const{
        return m_tap_stride;
    }


private:
    float* internal_tap_start(size_t tap_index){
        return m_data.data() + tap_index * m_tap_stride;
    }


private:
    size_t m_taps;
    size_t m_in_sample_rate;
    size_t m_out_sample_rate;
    size_t m_sample_block;
    size_t m_tap_block;
    size_t m_period_in;
    size_t m_period_out;
    size_t m_period_out_extended;
    size_t m_taps_allocated;
    size_t m_samples_allocated;
    size_t m_tap_stride;
    AlignedVector<uint32_t> m_index;
    AlignedVector<float> m_data;
};



}
}
}
#endif
