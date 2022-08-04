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
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSink.h"

namespace PokemonAutomation{



AudioSink::~AudioSink(){}

AudioSink::AudioSink(Logger& logger, const AudioDeviceInfo& device, AudioFormat format)
    : m_logger(logger)
{
    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();

    set_format(native_format, format);

    AudioSampleFormat stream_format = get_sample_format(native_format);
    if (stream_format == AudioSampleFormat::INVALID){
        stream_format = AudioSampleFormat::FLOAT32;
        setSampleFormatToFloat(native_format);
    }

    logger.log("AudioOutputDevice(): " + dumpAudioFormat(native_format));

    if (!native_info.isFormatSupported(native_format)){
        logger.log("Audio output device does not support the requested audio format.", COLOR_RED);
        return;
    }

    NativeAudioSink* sink = new NativeAudioSink(native_info, native_format);
    m_device.reset(sink);
    QIODevice* io_device = sink->start();

    switch (format){
    case AudioFormat::MONO_48000:
        m_sample_rate = 48000;
        m_channels = 1;
        m_multiplier = 1;
        m_writer.reset(new AudioSinkWriter(*io_device, stream_format, 1));
        break;
    case AudioFormat::DUAL_44100:
        m_sample_rate = 44100;
        m_channels = 2;
        m_multiplier = 1;
        m_writer.reset(new AudioSinkWriter(*io_device, stream_format, 2));
        break;
    case AudioFormat::DUAL_48000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        m_writer.reset(new AudioSinkWriter(*io_device, stream_format, 2));
        break;
    case AudioFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_sample_rate = 96000;
        m_channels = 1;
        m_multiplier = 2;
        m_writer.reset(new AudioSinkWriter(*io_device, stream_format, 2));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Interleaved format not allowed for audio output.");
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }
}

void AudioSink::set_volume(float volume){
    if (!m_device){
        return;
    }
    static_cast<NativeAudioSink&>(*m_device).setVolume(volume);
}




}
