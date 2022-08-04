/*  Audio Output Writer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSink.h"

namespace PokemonAutomation{




class AudioOutputDevice : public AudioFloatToStream, private StreamListener{
public:
    AudioOutputDevice(
        Logger& logger,
        const NativeAudioInfo& device, const QAudioFormat& format,
        AudioSampleFormat sample_format, size_t samples_per_frame
    )
        : AudioFloatToStream(nullptr, sample_format, samples_per_frame)
        , StreamListener(samples_per_frame * sample_size(sample_format))
        , m_sink(device, format)
    {
        m_io_device = m_sink.start();
        add_listener(*this);
    }
    ~AudioOutputDevice(){
        remove_listener(*this);
    }

    void set_volume(float volume){
        m_sink.setVolume(volume);
    }

    virtual void on_objects(const void* data, size_t objects) override{
        m_io_device->write((const char*)data, objects * object_size);
    }

private:
    NativeAudioSink m_sink;
    QIODevice* m_io_device;
};


AudioSink::~AudioSink(){}

AudioSink::AudioSink(Logger& logger, const AudioDeviceInfo& device, AudioFormat format)
    : m_logger(logger)
{
    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();

    set_format(native_format, format);

    AudioSampleFormat sample_format = get_sample_format(native_format);
    if (sample_format == AudioSampleFormat::INVALID){
        sample_format = AudioSampleFormat::FLOAT32;
        setSampleFormatToFloat(native_format);
    }

    logger.log("AudioOutputDevice(): " + dumpAudioFormat(native_format));
    if (!native_info.isFormatSupported(native_format)){
        logger.log("Audio output device does not support the requested audio format.", COLOR_RED);
        return;
    }

    switch (format){
    case AudioFormat::MONO_48000:
        m_sample_rate = 48000;
        m_channels = 1;
        m_multiplier = 1;
        break;
    case AudioFormat::DUAL_44100:
        m_sample_rate = 44100;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioFormat::DUAL_48000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_sample_rate = 96000;
        m_channels = 1;
        m_multiplier = 2;
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Interleaved format not allowed for audio output.");
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }

    m_writer = std::make_unique<AudioOutputDevice>(
        logger,
        native_info, native_format,
        sample_format, m_channels * m_multiplier
    );
}

AudioSink::operator AudioFloatStreamListener&(){
    return *m_writer;
}
void AudioSink::set_volume(float volume){
    if (!m_writer){
        return;
    }
    m_logger.log("Volume set to: " + tostr_default(volume));
    m_writer->set_volume(volume);
}




}
