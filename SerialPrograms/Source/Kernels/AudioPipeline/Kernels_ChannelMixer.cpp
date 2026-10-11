/*  Audio Pipeline - ChannelMixer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioChannelMixer/Kernels_AudioChannelMixer.h"
#include "Kernels_ChannelMixer.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



ChannelMixer::~ChannelMixer() = default;
ChannelMixer::ChannelMixer(
    size_t sample_rate,
    size_t in_channels,
    size_t out_channels,
    std::vector<float> weight_matrix,
    size_t buffer_size
)
    : WorkingAudioStreamListener(sample_rate, in_channels)
    , m_in_channels(in_channels)
    , m_out_channels(out_channels)
    , m_buffer_size(buffer_size)
    , m_weight_matrix(std::move(weight_matrix))
{}

void ChannelMixer::add_listener(WorkingAudioStreamListener& listener){
    if (sample_rate() != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (m_out_channels != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void ChannelMixer::remove_listener(WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}

void ChannelMixer::push_samples(size_t samples, size_t stride, const float* data){
    AudioChannelMixer::mix_channels(
        m_weight_matrix.data(),
        samples,
        m_out_channels, m_buffer_size, m_buffer.data(),
        m_in_channels, stride, data
    );
    m_listeners.run_method(
        &WorkingAudioStreamListener::push_samples,
        samples, m_buffer_size, m_buffer.data()
    );
}



}
}
}
