/*  Audio Pipeline - Kernels_WorkingAudio2RawAudio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "Kernels/AudioChannelInterleave/Kernels_AudioChannelInterleave.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels_WorkingAudio2RawAudio.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



template <typename Type>
size_t WorkingAudio2RawAudio::set_converter(){
    m_convert = [this](const void* data, size_t samples){
        Kernels::AudioStreamConverter::convert_to<Type>(
            (const float*)data, (Type*)m_interleaved_raw.data(), samples,
            m_multiplier.load(std::memory_order_acquire)
        );
    };
    return sizeof(Type);
}


WorkingAudio2RawAudio::~WorkingAudio2RawAudio() = default;
WorkingAudio2RawAudio::WorkingAudio2RawAudio(
    SampleFormat sample_type,
    size_t sample_rate,
    size_t channels,
    float multiplier,
    size_t buffer_size
)
    : WorkingAudioStreamListener(sample_rate, channels)
    , m_sample_type(sample_type)
    , m_multiplier(multiplier)
    , m_buffer_size(buffer_size)
{
    size_t sample_size = sizeof(float);
    switch (sample_type){
    case SampleFormat::UINT8:
        sample_size = set_converter<uint8_t>();
        break;
    case SampleFormat::SINT16:
        sample_size = set_converter<int16_t>();
        break;
    case SampleFormat::SINT32:
        sample_size = set_converter<int32_t>();
        break;
    case SampleFormat::FLOAT32:
        //  Float is already the working type, so the conversion is just the gain.
        sample_size = set_converter<float>();
        break;
    default:;
    }

    //  Round the buffer up to PA_ALIGNMENT bytes so that channels beyond
    //  channel 0 start aligned for the SIMD kernels.
    m_buffer_size = align_int_up<PA_ALIGNMENT>(buffer_size * sample_size) / sample_size;

    //  Only allocate the interleaved buffer if we need to convert types.
    if (m_convert != nullptr){
        m_interleaved_raw = AlignedVector<char>(channels * m_buffer_size * sample_size);
    }

    //  Only allocate the split buffer if we need to split channels.
    if (channels != 1){
        m_interleaved_floats = AlignedVector<float>(channels * m_buffer_size);
    }
}

void WorkingAudio2RawAudio::add_listener(RawAudioStreamListener& listener){
    if (m_sample_type != listener.sample_type()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample type doesn't match.");
    }
    if (sample_rate() != listener.sample_rate()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (channels() != listener.channels()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void WorkingAudio2RawAudio::remove_listener(RawAudioStreamListener& listener){
    m_listeners.remove(listener);
}
void WorkingAudio2RawAudio::set_multiplier(float multiplier){
    m_multiplier.store(multiplier, std::memory_order_release);
}

void WorkingAudio2RawAudio::push_samples(size_t samples, size_t stride, const float* data){
    //  Single-channel float pass-through. (Zero-copy, no gain.)
    if (m_sample_type == SampleFormat::FLOAT32 && channels() == 1 &&
        m_multiplier.load(std::memory_order_acquire) == 1.0f){
        m_listeners.run_method(
            &RawAudioStreamListener::push_samples,
            samples, (const float*)data
        );
        return;
    }

    while (samples > 0){
        size_t block = std::min(samples, m_buffer_size);

        //  Combine the channels. (if needed)
        const float* interleaved_floats;
        if (channels() != 1){
            Kernels::AudioChannelInterleave::channel_interleave(
                channels(), block,
                stride, data,
                m_interleaved_floats.data()
            );
            interleaved_floats = m_interleaved_floats.data();
        }else{
            interleaved_floats = data;
        }

        //  Convert to output type. (if needed)
        const void* interleaved_raw;
        if (m_convert){
            m_convert(interleaved_floats, block * channels());
            interleaved_raw = m_interleaved_raw.data();
        }else{
            interleaved_raw = interleaved_floats;
        }

        m_listeners.run_method(
            &RawAudioStreamListener::push_samples,
            block, interleaved_raw
        );

        samples -= block;
        data += block;
    }
}



}
}
}
