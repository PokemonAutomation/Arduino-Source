/*  Audio Input File
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include <QString>
#include "3rdParty/QtWavFile/WavFile.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioFileLoader.h"
#include "AudioInputFile.h"

namespace PokemonAutomation{



namespace{


Kernels::AudioPipeline::SampleFormat to_sample_format(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:   return Kernels::AudioPipeline::SampleFormat::UINT8;
    case AudioSampleFormat::SINT16:  return Kernels::AudioPipeline::SampleFormat::SINT16;
    case AudioSampleFormat::SINT32:  return Kernels::AudioPipeline::SampleFormat::SINT32;
    case AudioSampleFormat::FLOAT32: return Kernels::AudioPipeline::SampleFormat::FLOAT32;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported audio sample format.");
    }
}

Kernels::AudioPipeline::SampleFormat sample_format_of(const QAudioFormat& format){
    QAudioFormat copy = format;
    return to_sample_format(get_sample_format(copy));
}


}



bool probe_wav_native_format(const std::string& filename, QAudioFormat& out_format){
    WavFile wav_file;
    if (!wav_file.open(QString::fromStdString(filename))){
        return false;
    }
    out_format = wav_file.audioFormat();
    wav_file.close();
    if (out_format.channelCount() < 1 || out_format.sampleRate() < 1
        || out_format.sampleFormat() == QAudioFormat::SampleFormat::Unknown){
        return false;
    }
    return true;
}



AudioInputFile::AudioInputFile(
    Logger& logger, AudioStreamToFloat& reader,
    const std::string& file, const QAudioFormat& format
)
    : m_reader(reader)
    , m_source(std::make_unique<AudioFileLoader>(nullptr, file, format))
    , m_sample_type(sample_format_of(format))
    , m_sample_rate(format.sampleRate())
    , m_channels(format.channelCount())
    , m_frame_bytes(format.bytesPerFrame())
{
    logger.log("AudioInputFile(): " + dump_audio_format(format));
    connect(
        m_source.get(), &AudioFileLoader::bufferReady,
        this, [this](const char* data, size_t len){
            on_buffer_ready(data, len);
        }
    );
    m_source->start();
}
AudioInputFile::~AudioInputFile(){}

void AudioInputFile::add_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (listener.sample_type() != m_sample_type){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample type doesn't match.");
    }
    if (listener.sample_rate() != m_sample_rate){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (listener.channels() != m_channels){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void AudioInputFile::remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}

void AudioInputFile::on_buffer_ready(const char* data, size_t len){
    m_reader.push_bytes(data, len);

    //  AudioFileLoader only emits whole multiples of the frame size, so the
    //  raw listeners can take the data as-is.
    m_listeners.run_method(
        &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
        len / m_frame_bytes,
        data
    );
}



}
