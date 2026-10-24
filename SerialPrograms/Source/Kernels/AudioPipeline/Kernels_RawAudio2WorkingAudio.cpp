/*  Audio Pipeline - RawAudio2WorkingAudio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "Kernels/AudioChannelInterleave/Kernels_AudioChannelInterleave.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels_RawAudio2WorkingAudio.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



namespace{


size_t sample_format_size(SampleFormat sample_type){
    switch (sample_type){
    case SampleFormat::UINT8:   return sizeof(uint8_t);
    case SampleFormat::SINT16:  return sizeof(int16_t);
    case SampleFormat::SINT32:  return sizeof(int32_t);
    case SampleFormat::FLOAT32: return sizeof(float);
    default:;
    }
    return 0;
}


}



template <typename Type>
void RawAudio2WorkingAudio::set_converter(){
    m_convert = [this](const void* data, size_t samples){
        Kernels::AudioStreamConverter::convert_from<Type>(
            m_interleaved_floats.data(),
            (const Type*)data,
            samples,
            m_multiplier
        );
    };
}

RawAudio2WorkingAudio::~RawAudio2WorkingAudio() = default;
RawAudio2WorkingAudio::RawAudio2WorkingAudio(
    SampleFormat sample_type,
    size_t sample_rate,
    size_t channels,
    float multiplier,
    size_t buffer_size
)
    : RawAudioStreamListener(sample_type, sample_rate, channels)
    , m_multiplier(multiplier)
    , m_buffer_size(buffer_size)
{
    //  Round the buffer up to PA_ALIGNMENT bytes so that channels beyond
    //  channel 0 start aligned for the SIMD kernels.
    m_buffer_size =
        align_int_up<PA_ALIGNMENT>(buffer_size * sample_format_size(sample_type))
        / sample_format_size(sample_type);

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
    case SampleFormat::FLOAT32:
        //  Float is already the working type, so the conversion is just the gain.
        set_converter<float>();
        break;
    default:;
    }

    //  Only allocate the interleaved buffer if we need to convert types.
    if (m_convert != nullptr){
        m_interleaved_floats = AlignedVector<float>(channels * m_buffer_size);
    }

    //  Only allocate the split buffer if we need to split channels.
    if (channels != 1){
        m_split_floats = AlignedVector<float>(channels * m_buffer_size);
    }
}

void RawAudio2WorkingAudio::validate_listener(const WorkingAudioStreamListener& listener) const{
    if (sample_rate() != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (channels() != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
}

void RawAudio2WorkingAudio::add_listener(WorkingAudioStreamListener& listener){
    validate_listener(listener);
    m_listeners.add(listener);
}
void RawAudio2WorkingAudio::push_front_listener(WorkingAudioStreamListener& listener){
    validate_listener(listener);
    m_listeners.push_front(listener);
}
void RawAudio2WorkingAudio::remove_listener(WorkingAudioStreamListener& listener){
    m_listeners.remove(listener);
}
void RawAudio2WorkingAudio::set_multiplier(float multiplier){
    m_multiplier.store(multiplier, std::memory_order_release);
}

void RawAudio2WorkingAudio::push_samples(size_t frames, const void* data){
    //  Single-channel float pass-through. (Zero-copy, no gain.)
    if (sample_type() == SampleFormat::FLOAT32 && channels() == 1 &&
        m_multiplier.load(std::memory_order_acquire) == 1.0f){
        m_listeners.run_method(
            &WorkingAudioStreamListener::push_samples,
            frames, 0, (const float*)data
        );
        return;
    }

    const size_t sample_size = sample_format_size(sample_type());
    while (frames > 0){
        size_t block = std::min(frames, m_buffer_size);

        //  Convert to float. (if needed)
        const float* interleaved_floats;
        if (m_convert){
            m_convert(data, block * channels());
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
        data = (const char*)data + block * channels() * sample_size;
    }
}



}
}
}
