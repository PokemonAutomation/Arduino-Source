/*  Audio Pipeline - AudioChannelInterleaver
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_AudioChannelInterleaver.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



AudioChannelInterleaver::~AudioChannelInterleaver() = default;
AudioChannelInterleaver::AudioChannelInterleaver(
    size_t in_sample_rate,
    size_t buffer_size
)
    : WorkingAudioStreamListener(in_sample_rate, 2)
    , m_in_rate(in_sample_rate)
    , m_buffer_size(buffer_size)
    , m_buffer(2 * buffer_size)
{
}

void AudioChannelInterleaver::add_listener(WorkingAudioStreamListener& listener){
    if (listener.sample_rate() != out_sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (listener.channels() != 1){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void AudioChannelInterleaver::remove_listener(WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}

void AudioChannelInterleaver::push_samples(size_t samples, size_t stride, const float* data){
    //  Channel 0 takes the even positions, channel 1 the odd positions.
    float* out = m_buffer.data();
    for (size_t s = 0; s < samples; s++){
        out[2 * s + 0] = data[s];
        out[2 * s + 1] = data[stride + s];
    }
    m_listeners.run_method(
        &WorkingAudioStreamListener::push_samples,
        2 * samples, 2 * m_buffer_size, m_buffer.data()
    );
}



}
}
}
