/*  Audio Input Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstring>
#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioInputDevice.h"

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


}



AudioInputDevice::AudioInputDevice(
    Logger& logger, AudioStreamToFloat& reader,
    const NativeAudioInfo& device, const QAudioFormat& format
)
    : MisalignedStreamConverter(
        format.bytesPerFrame(), format.bytesPerFrame(),
        RAW_BUFFER_CAPACITY
      )
    , ObjectStreamListener(format.bytesPerFrame())
    , m_reader(reader)
    , m_sample_type(sample_format_of(format))
    , m_sample_rate(format.sampleRate())
    , m_channels(format.channelCount())
    , m_frame_bytes(format.bytesPerFrame())
{
    MisalignedStreamConverter::add_listener(*this);

    logger.log("AudioInputDevice(): " + dump_audio_format(format));
    if (!device.isFormatSupported(format)){
//            throw InternalProgramError(&logger, PA_CURRENT_FUNCTION, "Format not supported: " + dump_audio_format(format));
        logger.log("Format not supported: " + dump_audio_format(format), COLOR_RED);
        return;
    }
    m_source = std::make_unique<NativeAudioSource>(device, format);

    this->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    WallClock start = current_time();
    m_source->start(this);
    WallClock end = current_time();
    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
    logger.log("Done starting audio... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);
}
AudioInputDevice::~AudioInputDevice(){
    if (m_source){
        m_source->stop();
    }
}

void AudioInputDevice::add_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
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
void AudioInputDevice::remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}

qint64 AudioInputDevice::writeData(const char* data, qint64 len){
    m_reader.push_bytes(data, len);
    MisalignedStreamConverter::push_bytes(data, len);
    return len;
}

void AudioInputDevice::convert(void* out, const void* in, size_t count){
    //  Raw pass-through: the input and output frame sizes are equal.
    std::memcpy(out, in, count * m_frame_bytes);
}

void AudioInputDevice::on_objects(const void* data, size_t objects){
    m_listeners.run_method(
        &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
        objects, data
    );
}



}
