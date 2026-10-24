/*  Audio Output Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "AudioOutputDevice.h"

namespace PokemonAutomation{

namespace{


const char* audio_error_to_str(QAudio::Error error){
    switch (error){
    case QAudio::NoError:
        return "NoError";
    case QAudio::OpenError:
        return "OpenError (device cannot be opened with the requested format)";
    case QAudio::IOError:
        return "IOError";
    case QAudio::UnderrunError:
        return "UnderrunError";
    case QAudio::FatalError:
        return "FatalError";
    default:
        return "UnknownError";
    }
}


size_t raw_sample_size(Kernels::AudioPipeline::SampleFormat format){
    switch (format){
    case Kernels::AudioPipeline::SampleFormat::UINT8:   return sizeof(uint8_t);
    case Kernels::AudioPipeline::SampleFormat::SINT16:  return sizeof(int16_t);
    case Kernels::AudioPipeline::SampleFormat::SINT32:  return sizeof(int32_t);
    case Kernels::AudioPipeline::SampleFormat::FLOAT32: return sizeof(float);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported audio sample format.");
    }
}


}



AudioOutputDevice::AudioOutputDevice(
    Logger& logger,
    const NativeAudioInfo& device,
    const QAudioFormat& format,
    Kernels::AudioPipeline::SampleFormat sample_format,
    size_t sample_rate,
    size_t channels
)
    : Kernels::AudioPipeline::RawAudioStreamListener(sample_format, sample_rate, channels)
    , m_bytes_per_frame(channels * raw_sample_size(sample_format))
    , m_logger(logger)
    , m_sink(device, format)
    , m_io_device(nullptr)
{
    m_sink.connect(
        &m_sink, &QAudioSink::stateChanged,
        &m_sink, [this](QAudio::State state){
            if (state == QAudio::State::StoppedState){
                m_io_device = nullptr;
                QAudio::Error error = m_sink.error();
                if (error == QAudio::NoError){
                    m_logger.log("AudioOutputDevice has stopped.", COLOR_ORANGE);
                }else{
                    m_logger.log(
                        std::string("AudioOutputDevice has stopped with error: ") + audio_error_to_str(error),
                        COLOR_RED
                    );
                }
            }
        }
    );

    m_io_device = m_sink.start();
    QAudio::Error error = m_sink.error();
    if (m_io_device == nullptr || error != QAudio::NoError){
        m_logger.log(
            std::string("Unable to start audio output device: ") + audio_error_to_str(error),
            COLOR_RED
        );
    }
}
AudioOutputDevice::~AudioOutputDevice(){}

void AudioOutputDevice::push_samples(size_t frames, const void* data){
    auto scope_check = m_sanitizer.check_scope();
    if (m_io_device != nullptr){
        m_io_device->write((const char*)data, frames * m_bytes_per_frame);
    }
}



}
