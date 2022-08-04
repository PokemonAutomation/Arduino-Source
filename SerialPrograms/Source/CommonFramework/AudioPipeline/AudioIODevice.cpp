/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "AudioConstants.h"
#include "AudioIODevice.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




AudioIODevice::~AudioIODevice(){}

AudioIODevice::AudioIODevice(Logger& logger, const std::string& file, AudioFormat our_format, AudioSampleFormat input_format)
     : m_format(our_format)
     , m_reader(new AudioInputReader(logger, file, our_format))
{
    make_FFT_runner();

    //  Register the FFT runner so that it receives data samples from the audio reader.
    m_reader->add_listener(*m_fft_runner);
}
AudioIODevice::AudioIODevice(Logger& logger, const AudioDeviceInfo& device, AudioFormat our_format, AudioSampleFormat input_format)
     : m_format(our_format)
     , m_reader(new AudioInputReader(logger, device, our_format))
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


void AudioIODevice::setAudioSinkDevice(QIODevice* audioSinkDevice, AudioSampleFormat output_format){
    if (m_output){
        m_reader->remove_listener(*m_output);
    }
    m_audioSinkDevice = audioSinkDevice;
    if (audioSinkDevice == nullptr){
        return;
    }
    switch (m_format){
    case AudioFormat::MONO_48000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 1));
        break;
    case AudioFormat::DUAL_44100:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 2));
        break;
    case AudioFormat::DUAL_48000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 2));
        break;
    case AudioFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 2));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 2));
        break;
    case AudioFormat::INTERLEAVE_RL_96000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, output_format, 2));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
    m_reader->add_listener(*m_output);
}








}
