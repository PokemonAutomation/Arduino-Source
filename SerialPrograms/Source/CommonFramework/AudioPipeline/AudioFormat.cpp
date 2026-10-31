/*  Audio Format
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioFormat>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "AudioFormat.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



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
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "Invalid AudioSampleFormat: " + std::to_string((size_t)format)
        );
    }
}


AudioSampleFormat sample_format_of(const QAudioFormat& format){
    switch (format.sampleFormat()){
    case QAudioFormat::SampleFormat::Float:
        return AudioSampleFormat::FLOAT32;
    case QAudioFormat::SampleFormat::UInt8:
        return AudioSampleFormat::UINT8;
    case QAudioFormat::SampleFormat::Int16:
        return AudioSampleFormat::SINT16;
    case QAudioFormat::SampleFormat::Int32:
        return AudioSampleFormat::SINT32;
    default:
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "Unsupported audio sample format."
        );
    }
}



AudioFormat::AudioFormat()
    : m_native_format(CONSTRUCT_TOKEN)
{}
AudioFormat::AudioFormat(QAudioFormat native_format)
    : m_native_format(CONSTRUCT_TOKEN, std::move(native_format))
{}

AudioSampleFormat AudioFormat::sample_format() const{
    return sample_format_of(*m_native_format);
}
size_t AudioFormat::sample_rate() const{
    return m_native_format->sampleRate();
}
size_t AudioFormat::channels() const{
    return m_native_format->channelCount();
}

const QAudioFormat& AudioFormat::native_format() const{
    return *m_native_format;
}
QAudioFormat& AudioFormat::native_format(){
    return *m_native_format;
}



}
