/*  Audio Output Writer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5
#include <QAudioOutput>
using NativeAudioSink = QAudioOutput;
#elif QT_VERSION_MAJOR == 6
#include <QAudioSink>
using NativeAudioSink = QAudioSink;
#endif

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSink.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



// Slider bar volume: [0, 100], in log scale
// Volume value passed to AudioDisplayWidget (and the audio thread it manages): [0.f, 1.f], linear scale
float convertAudioVolumeFromSlider(double volume){
    volume = std::max(volume, 0.0);
    volume = std::min(volume, 1.0);
    // The slider bar value is in the log scale because log scale matches human sound
    // perception.
    float linearVolume = QAudio::convertVolume(
        (float)volume,
        QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale
    );
    return linearVolume;
}




class AudioOutputDevice : public AudioFloatToStream, private StreamListener{
public:
    AudioOutputDevice(
        Logger& logger,
        const NativeAudioInfo& device, const QAudioFormat& format,
        AudioSampleFormat sample_format, size_t samples_per_frame,
        double volume
    )
        : AudioFloatToStream(sample_format, samples_per_frame)
        , StreamListener(samples_per_frame * sample_size(sample_format))
        , m_logger(logger)
        , m_sink(device, format)
    {
        m_sink.connect(
            &m_sink, &NativeAudioSink::stateChanged,
            &m_sink, [this](QAudio::State state){
                if (state == QAudio::State::StoppedState){
                    m_io_device = nullptr;
                    m_logger.log("AudioOutputDevice has stopped.", COLOR_ORANGE);
                }
            }
        );

        m_io_device = m_sink.start();
        m_sink.setVolume(convertAudioVolumeFromSlider(volume));
        add_listener(*this);
    }
    ~AudioOutputDevice(){
        remove_listener(*this);
    }

    void set_volume(double volume){
        auto scope_check = m_sanitizer.check_scope();
        double absolute = convertAudioVolumeFromSlider(volume);
        m_logger.log("Volume set to: Slider = " + tostr_default(volume) + " -> Absolute = " + tostr_default(absolute));
        m_sink.setVolume(absolute);
    }

    virtual void on_objects(const void* data, size_t objects) override{
        auto scope_check = m_sanitizer.check_scope();
        if (m_io_device != nullptr){
            m_io_device->write((const char*)data, objects * object_size);
        }
    }

private:
    Logger& m_logger;
    NativeAudioSink m_sink;
    QIODevice* m_io_device;
    LifetimeSanitizer m_sanitizer;
};


AudioSink::~AudioSink(){}

AudioSink::AudioSink(Logger& logger, const AudioDeviceInfo& device, AudioChannelFormat format, double volume){
    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();
    QAudioFormat target_format = native_format;

    set_format(target_format, format);

    AudioSampleFormat sample_format = get_sample_format(target_format);
    if (sample_format == AudioSampleFormat::INVALID){
        sample_format = AudioSampleFormat::FLOAT32;
        setSampleFormatToFloat(target_format);
    }

    logger.log("AudioOutputDevice(): Target: " + dumpAudioFormat(target_format));
    logger.log("AudioOutputDevice(): Native: " + dumpAudioFormat(native_format));
    if (!native_info.isFormatSupported(native_format) &&
        native_format != target_format
    ){
        logger.log("Audio output device does not support the requested audio format.", COLOR_RED);
        return;
    }

    switch (format){
    case AudioChannelFormat::MONO_48000:
        m_sample_rate = 48000;
        m_channels = 1;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::DUAL_44100:
        m_sample_rate = 44100;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::DUAL_48000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_sample_rate = 96000;
        m_channels = 1;
        m_multiplier = 2;
        break;
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Interleaved format not allowed for audio output.");
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }

    m_writer = std::make_unique<AudioOutputDevice>(
        logger,
        native_info, target_format,
        sample_format, m_channels * m_multiplier,
        volume
    );
}

AudioFloatStreamListener* AudioSink::float_stream_listener(){
    auto scope_check = m_sanitizer.check_scope();
    return m_writer.get();
}
void AudioSink::set_volume(double volume){
    auto scope_check = m_sanitizer.check_scope();
    if (!m_writer){
        return;
    }
    m_writer->set_volume(volume);
}




}
