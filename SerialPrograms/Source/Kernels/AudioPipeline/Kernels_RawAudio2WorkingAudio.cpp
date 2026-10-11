/*  Audio Pipeline - RawAudio2WorkingAudio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "Kernels/AudioChannelInterleave/Kernels_AudioChannelInterleave.h"
#include "Kernels_RawAudio2WorkingAudio.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



template <typename Type>
void RawAudio2WorkingAudio::set_converter(){
    m_convert = [this](const void* data, size_t samples){
        Kernels::AudioStreamConverter::convert_from<Type>(
            m_interleaved_floats.data(), (const Type*)data, samples
        );
    };
}

RawAudio2WorkingAudio::~RawAudio2WorkingAudio() = default;
RawAudio2WorkingAudio::RawAudio2WorkingAudio(
    SampleFormat sample_type,
    size_t sample_rate,
    size_t channels,
    size_t buffer_size
)
    : RawAudioStreamListener(sample_type, sample_rate, channels)
    , m_buffer_size(buffer_size)
{
    //  Single-channel float is pass-through.
    if (sample_type == SampleFormat::FLOAT32 && channels == 1){
        return;
    }

    switch (sample_type){
    case SampleFormat::UINT8:
        set_converter<uint8_t>();
        break;
    case SampleFormat::SINT16:
        set_converter<int16_t>();
        break;
    case SampleFormat::SINT32:
        set_converter<int32_t>();
        break;
    default:;
    }

    //  Only allocate the interleaved buffer if we need to convert types.
    if (m_convert != nullptr){
        m_interleaved_floats = AlignedVector<float>(channels * buffer_size);
    }

    //  Only allocate the split buffer if we need to split channels.
    if (channels != 1){
        m_split_floats = AlignedVector<float>(channels * buffer_size);
    }
}

void RawAudio2WorkingAudio::add_listener(WorkingAudioStreamListener& listener){
    if (sample_rate() != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (channels() != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void RawAudio2WorkingAudio::remove_listener(WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}

void RawAudio2WorkingAudio::push_samples(size_t frames, const void* data){
    //  Single-channel float pass-through.
    if (sample_type() == SampleFormat::FLOAT32 && channels() == 1){
        m_listeners.run_method(
            &WorkingAudioStreamListener::push_samples,
            frames, 0, (const float*)data
        );
        return;
    }

    while (frames > 0){
        size_t block = std::min(frames, m_buffer_size);

        //  Convert to float. (if needed)
        const float* interleaved_floats;
        if (m_convert){
            m_convert(data, block);
            interleaved_floats = m_interleaved_floats.data();
        }else{
            interleaved_floats = (const float*)data;
        }

        //  Now split the channels. (if needed)
        const float* split_floats;
        if (channels() != 1){
            Kernels::AudioChannelInterleave::channel_deinterleave(
                channels(), block,
                m_buffer_size, m_split_floats.data(),
                interleaved_floats
            );
            split_floats = m_split_floats.data();
        }else{
            split_floats = interleaved_floats;
        }

        m_listeners.run_method(
            &WorkingAudioStreamListener::push_samples,
            block, m_buffer_size, split_floats
        );

        frames -= block;
    }
}



}
}
}
