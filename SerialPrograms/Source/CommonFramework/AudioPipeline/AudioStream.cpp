/*  Audio Source Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/AudioStreamConversion/AudioStreamConversion.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "AudioConstants.h"
#include "AudioStream.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


//  Audio buffer size (measured in frames).
const size_t AUDIO_BUFFER_SIZE = 4096;



size_t sample_size(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:
        return sizeof(uint8_t);
    case AudioSampleFormat::SINT16:
        return sizeof(int16_t);
    case AudioSampleFormat::SINT32:
        return sizeof(int32_t);
    case AudioSampleFormat::FLOAT32:
        return sizeof(float);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioSampleFormat: " + std::to_string((size_t)format));
    }
}



void AudioStreamToFloat::add_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (listener.expected_samples_per_frame != m_samples_per_frame){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching frame size.");
    }
    m_listeners.insert(&listener);
}
void AudioStreamToFloat::remove_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.erase(&listener);
}

AudioStreamToFloat::~AudioStreamToFloat(){
    MisalignedStreamConverter::remove_listener(*this);
}
AudioStreamToFloat::AudioStreamToFloat(
    AudioSampleFormat input_format,
    size_t samples_per_frame,
    float volume_multiplier,
    bool reverse_channels
)
    : MisalignedStreamConverter(
        sample_size(input_format) * samples_per_frame,
        sizeof(float) * samples_per_frame,
        AUDIO_BUFFER_SIZE
    )
    , StreamListener(sizeof(float) * samples_per_frame)
    , m_format(input_format)
    , m_samples_per_frame(samples_per_frame)
    , m_volume_multiplier(volume_multiplier)
    , m_reverse_channels(reverse_channels)
    , m_sample_size(sample_size(input_format))
    , m_frame_size(m_sample_size * samples_per_frame)
{
    if (samples_per_frame == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Must have at least one sample.");
    }
    if (reverse_channels && samples_per_frame != 2){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Reverse channels only works with 2 samples/frame.");
    }
    MisalignedStreamConverter::add_listener(*this);
}
void AudioStreamToFloat::on_objects(const void* data, size_t objects){
    auto scope_check = m_sanitizer.check_scope();
    for (AudioFloatStreamListener* listener : m_listeners){
        listener->on_samples((const float*)data, objects);
    }
}
void AudioStreamToFloat::convert(void* out, const void* in, size_t count){
    auto scope_check = m_sanitizer.check_scope();
    switch (m_format){
    case AudioSampleFormat::UINT8:
        Kernels::AudioStreamConversion::convert_audio_uint8_to_float(
            (float*)out, (const uint8_t*)in, count * m_samples_per_frame, m_volume_multiplier
        );
        break;
    case AudioSampleFormat::SINT16:
        Kernels::AudioStreamConversion::convert_audio_sint16_to_float(
            (float*)out, (const int16_t*)in, count * m_samples_per_frame, m_volume_multiplier
        );
        break;
    case AudioSampleFormat::SINT32:
        Kernels::AudioStreamConversion::convert_audio_sint32_to_float(
            (float*)out, (const int32_t*)in, count * m_samples_per_frame, m_volume_multiplier
        );
        break;
    case AudioSampleFormat::FLOAT32:
        if (m_volume_multiplier == 1.0){
            memcpy(out, in, count * m_frame_size);
        }else{
            float volume_multiplier = m_volume_multiplier;
            float* o = (float*)out;
            const float* i = (float*)in;
            size_t stop = count * m_samples_per_frame;
            for (size_t c = 0; c < stop; c++){
                o[c] = i[c] * volume_multiplier;
            }
        }
        break;
    case AudioSampleFormat::INVALID:
        break;
    }
    if (m_reverse_channels){
        float* ptr = (float*)out;
        for (size_t c = 0; c < count; c++){
            float r0 = ptr[2*c + 0];
            float r1 = ptr[2*c + 1];
            ptr[2*c + 0] = r1;
            ptr[2*c + 1] = r0;
        }
    }
}




void AudioFloatToStream::add_listener(StreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (listener.object_size != m_frame_size){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching frame size.");
    }
    m_listeners.add(listener);
}
void AudioFloatToStream::remove_listener(StreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}

AudioFloatToStream::AudioFloatToStream(AudioSampleFormat output_format, size_t samples_per_frame)
    : AudioFloatStreamListener(samples_per_frame)
    , m_format(output_format)
    , m_samples_per_frame(samples_per_frame)
    , m_sample_size(sample_size(output_format))
    , m_frame_size(m_sample_size * samples_per_frame)
    , m_buffer_size(AUDIO_BUFFER_SIZE)
{
    switch (output_format){
    case AudioSampleFormat::INVALID:
    case AudioSampleFormat::FLOAT32:
        break;
    default:
        m_buffer = AlignedVector<char>(m_frame_size * m_buffer_size);
    }
}
AudioFloatToStream::~AudioFloatToStream(){}
void AudioFloatToStream::on_samples(const float* data, size_t frames){
    auto scope_check = m_sanitizer.check_scope();
    if (m_format == AudioSampleFormat::INVALID){
        return;
    }
    if (m_format == AudioSampleFormat::FLOAT32){
        m_listeners.run_method_unique(
            &StreamListener::on_objects,
            data, frames
        );
        return;
    }

    while (frames > 0){
        size_t block = std::min(frames, m_buffer_size);
        switch (m_format){
        case AudioSampleFormat::UINT8:
            Kernels::AudioStreamConversion::convert_audio_float_to_uint8((uint8_t*)m_buffer.data(), data, block * m_samples_per_frame);
            break;
        case AudioSampleFormat::SINT16:
            Kernels::AudioStreamConversion::convert_audio_float_to_sint16((int16_t*)m_buffer.data(), data, block * m_samples_per_frame);
            break;
        case AudioSampleFormat::SINT32:
            Kernels::AudioStreamConversion::convert_audio_float_to_sint32((int32_t*)m_buffer.data(), data, block * m_samples_per_frame);
            break;
        case AudioSampleFormat::FLOAT32:
            memcpy(m_buffer.data(), data, block * m_frame_size);
            break;
        default:
            return;
        }
        m_listeners.run_method_unique(
            &StreamListener::on_objects,
            m_buffer.data(), block
        );
        data = (const float*)((const char*)data + block * m_frame_size);
        frames -= block;
    }
}








}
