/*  Audio Pipeline - ChannelMixer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioChannelMixer/Kernels_AudioChannelMixer.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels_ChannelMixer.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



ChannelMixer::~ChannelMixer() = default;
ChannelMixer::ChannelMixer(
    Logger& logger,
    size_t sample_rate,
    size_t in_channels,
    size_t out_channels,
    std::vector<float> weight_matrix,
    size_t buffer_size
)
    : WorkingAudioNode(sample_rate, sample_rate, in_channels, out_channels)
    //  Round the buffer up to PA_ALIGNMENT bytes so that channels beyond
    //  channel 0 start aligned for the SIMD kernels.
    , m_buffer_size(
        align_int_up<PA_ALIGNMENT>(buffer_size * sizeof(float)) / sizeof(float)
    )
    , m_weight_matrix(std::move(weight_matrix))
    , m_buffer(out_channels * m_buffer_size)
{
    if (m_weight_matrix.size() != in_channels * out_channels){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Weight matrix size doesn't match.");
    }
    logger.log(
        "Initializing ChannelMixer: " + tostr_u_commas(in_channels) +
        " -> " + tostr_u_commas(out_channels) +
        " x " + tostr_u_commas(sample_rate) + " Hz",
        COLOR_PURPLE
    );
}

void ChannelMixer::add_listener(WorkingAudioStreamListener& listener){
    if (out_sample_rate() != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (out_channels() != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void ChannelMixer::remove_listener(WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}

void ChannelMixer::push_samples(size_t samples, size_t stride, const float* data){
    //  mix_channels() strides the output buffer at m_buffer_size, so it can
    //  mix at most m_buffer_size samples per call; chunk larger pushes.
    while (samples > 0){
        const size_t block = std::min(samples, m_buffer_size);
        AudioChannelMixer::mix_channels(
            m_weight_matrix.data(),
            block,
            out_channels(), m_buffer_size, m_buffer.data(),
            in_channels(), stride, data
        );
        m_listeners.run_method(
            &WorkingAudioStreamListener::push_samples,
            block, m_buffer_size, m_buffer.data()
        );
        data += block;
        samples -= block;
    }
}



}
}
}
