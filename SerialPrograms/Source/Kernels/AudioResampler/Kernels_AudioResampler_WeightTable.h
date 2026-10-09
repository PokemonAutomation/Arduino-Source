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
        size_t min_samples
    );

    size_t taps() const{
        return m_taps;
    }
    size_t period() const{
        return m_period;
    }
    const uint32_t* in_sample_index() const{
        return m_index.data();
    }
    const float* tap_start(size_t tap_index) const{
        return m_data.data() + tap_index * m_tap_stride;
    }


private:
    float* internal_tap_start(size_t tap_index){
        return m_data.data() + tap_index * m_tap_stride;
    }


private:
    size_t m_taps;
    size_t m_period;
    size_t m_tap_stride;
    AlignedVector<uint32_t> m_index;
    AlignedVector<float> m_data;
};



}
}
}
#endif
