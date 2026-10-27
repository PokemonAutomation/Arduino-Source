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
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioInputDevice.h"

namespace PokemonAutomation{



namespace{


//  Frames of raw audio buffered before the raw listeners are notified.
const size_t RAW_BUFFER_CAPACITY = 4096;


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



AudioInputDevice::AudioInputDevice(
    Logger& logger,
    const NativeAudioInfo& device, const QAudioFormat& format,
    bool reinterpret_96k_mono_as_stereo
)
    : MisalignedStreamConverter(
        raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo),
        raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo),
        RAW_BUFFER_CAPACITY
      )
    , ObjectStreamListener(raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo))
    , m_sample_type(sample_format_of(format))
    , m_sample_rate(format.sampleRate())
    , m_channels(format.channelCount())
    , m_frame_bytes(format.bytesPerFrame())
    , m_out_sample_rate(is_96k_mono_reinterpret(format, reinterpret_96k_mono_as_stereo) ? 48000 : format.sampleRate())
    , m_out_channels(is_96k_mono_reinterpret(format, reinterpret_96k_mono_as_stereo) ? 2 : format.channelCount())
    , m_out_frame_bytes(raw_output_frame_bytes(format, reinterpret_96k_mono_as_stereo))
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
    if (listener.sample_rate() != m_out_sample_rate){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Sample rate doesn't match.");
    }
    if (listener.channels() != m_out_channels){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Channel count doesn't match.");
    }
    m_listeners.add(listener);
}
void AudioInputDevice::remove_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}

qint64 AudioInputDevice::writeData(const char* data, qint64 len){
    MisalignedStreamConverter::push_bytes(data, len);
    return len;
}

void AudioInputDevice::convert(void* out, const void* in, size_t count){
    //  Raw pass-through: the input and output frame sizes are equal.
    std::memcpy(out, in, count * m_out_frame_bytes);
}

void AudioInputDevice::on_objects(const void* data, size_t objects){
    m_listeners.run_method(
        &Kernels::AudioPipeline::RawAudioStreamListener::push_samples,
        objects, data
    );
}



}
