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
        //  Extend the table to at least this many samples even if it goes
        //  beyond the period. Block kernels may overshoot the period.
        size_t min_samples,
        //  Extend the tap count to this many taps with zero padding.
        //  Block kernels may overshoot the tap count.
        size_t min_taps
    );

    size_t taps() const{
        return m_taps;
    }
    //  Output samples per period (out_sample_rate / gcd).
    size_t period_out() const{
        return m_period_out;
    }
    //  Input samples per output period (in_sample_rate / gcd).
    size_t period_in() const{
        return m_period_in;
    }
    //  Tap rows in the table (the requested taps, zero-padded to
    //  min_taps).
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
    size_t m_period_out;
    size_t m_period_in;
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
