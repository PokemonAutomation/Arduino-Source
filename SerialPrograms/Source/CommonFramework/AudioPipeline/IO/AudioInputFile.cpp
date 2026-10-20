/*  Audio Input File
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstring>
#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioFileLoader.h"
#include "AudioInputFile.h"

namespace PokemonAutomation{



namespace{


//  Frames of raw audio buffered before the raw listeners are notified.
//  (Matches AudioStreamToFloat's AUDIO_BUFFER_SIZE.)
const size_t RAW_BUFFER_CAPACITY = 4096;


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


//  When the preset says the physical 1ch x 96000 stream is really the
//  card's two 48 kHz channels, the raw listener output is that same byte
//  stream reinterpreted as 2ch x 48000 (aligned to 2-sample stereo-frame
//  boundaries). Every other case passes through with the physical frame
//  size.
bool is_96k_mono_reinterpret(const QAudioFormat& format, bool reinterpret_96k_mono_as_stereo){
    return reinterpret_96k_mono_as_stereo
        && format.channelCount() == 1 && format.sampleRate() == 96000;
}

size_t raw_output_frame_bytes(const QAudioFormat& format, bool reinterpret_96k_mono_as_stereo){
    return format.bytesPerFrame() * (is_96k_mono_reinterpret(format, reinterpret_96k_mono_as_stereo) ? 2 : 1);
}


}



AudioInputFile::AudioInputFile(
    Logger& logger, AudioStreamToFloat& reader,
    const std::string& file, const QAudioFormat& format,
    bool reinterpret_96k_mono_as_stereo
)
    : MisalignedStreamConverter(
        raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo),
        raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo),
        RAW_BUFFER_CAPACITY
      )
    , ObjectStreamListener(raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo))
    , m_reader(reader)
    , m_source(std::make_unique<AudioFileLoader>(nullptr, file, format))
    , m_sample_type(sample_format_of(format))
    , m_sample_rate(format.sampleRate())
    , m_channels(format.channelCount())
    , m_frame_bytes(format.bytesPerFrame())
    , m_out_sample_rate(is_96k_mono_reinterpret(format, reinterpret_96k_mono_as_stereo) ? 48000 : format.sampleRate())
    , m_out_channels(is_96k_mono_reinterpret(format, reinterpret_96k_mono_as_stereo) ? 2 : format.channelCount())
    , m_out_frame_bytes(raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo))
{
    MisalignedStreamConverter::add_listener(*this);

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
    if (listener.sample_rate() != m_out_sample_rate){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (listener.channels() != m_out_channels){
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

    if (m_out_frame_bytes != m_frame_bytes){
        //  96 kHz reinterpretation: align the listener output to 2-sample
        //  stereo-frame boundaries.
        MisalignedStreamConverter::push_bytes(data, len);
    }else{
        //  AudioFileLoader only emits whole multiples of the frame size, so
        //  the raw listeners can take the data as-is.
        m_listeners.run_method(
            &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
            len / m_frame_bytes,
            data
        );
    }
}


void AudioInputFile::convert(void* out, const void* in, size_t count){
    //  Raw pass-through: the input and output frame sizes are equal.
    std::memcpy(out, in, count * m_out_frame_bytes);
}

void AudioInputFile::on_objects(const void* data, size_t objects){
    m_listeners.run_method(
        &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
        objects, data
    );
}



}
