/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QtEndian>
using AudioSource = QAudioInput;
using AudioSink = QAudioOutput;
#elif QT_VERSION_MAJOR == 6
#include <QMediaDevices>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioDevice>
using AudioSource = QAudioSource;
using AudioSink = QAudioSink;
#endif

#include "Common/Cpp/Exceptions.h"
#include "AudioConstants.h"
#include "Tools/AudioFormatUtils.h"
#include "IO/AudioSink.h"
#include "AudioIODevice.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




AudioIODevice::~AudioIODevice(){}

AudioIODevice::AudioIODevice(Logger& logger, const std::string& file, AudioFormat our_format, AudioSampleFormat input_format)
     : m_format(our_format)
     , m_reader(new AudioSource(logger, file, our_format))
{
    make_FFT_runner();

    //  Register the FFT runner so that it receives data samples from the audio reader.
    m_reader->add_listener(*m_fft_runner);
}
AudioIODevice::AudioIODevice(Logger& logger, const AudioDeviceInfo& device, AudioFormat our_format, AudioSampleFormat input_format)
     : m_format(our_format)
     , m_reader(new AudioSource(logger, device, our_format))
{
    make_FFT_runner();

    //  Register the FFT runner so that it receives data samples from the audio reader.
    m_reader->add_listener(*m_fft_runner);
}

void AudioIODevice::make_FFT_runner(){
    switch (m_format){
    case AudioFormat::MONO_48000:
        m_fft_runner.reset(new FFTRunner(*this, 48000, 1, false));
        break;
    case AudioFormat::DUAL_44100:
        m_fft_runner.reset(new FFTRunner(*this, 44100, 2, true));
        break;
    case AudioFormat::DUAL_48000:
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_RL_96000:
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
}


void AudioIODevice::setAudioSinkDevice(std::unique_ptr<AudioSink> writer){
    if (m_writer){
        m_reader->remove_listener(*m_writer);
        m_writer.reset();
    }
    if (writer){
        m_writer = std::move(writer);
        m_reader->add_listener(*m_writer);
    }
}

void AudioIODevice::set_volume(float volume){
    if (m_writer){
        m_writer->set_volume(volume);
    }
}







}
