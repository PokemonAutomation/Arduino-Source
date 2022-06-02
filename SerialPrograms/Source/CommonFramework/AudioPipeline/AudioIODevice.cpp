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
AudioIODevice::AudioIODevice(AudioFormat format)
     : QIODevice(nullptr)
     , m_format(format)
{
    switch (format){
    case AudioFormat::MONO_48000:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 1, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 1, false));
        break;
    case AudioFormat::DUAL_44100:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 44100, 2, true));
        break;
    case AudioFormat::DUAL_48000:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::MONO_96000:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_RL_96000:
        m_reader.reset(new AudioSourceReader(AudioStreamFormat::FLOAT32, 2, true));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
    *m_reader += *m_fft_runner;
}
void AudioIODevice::setAudioSinkDevice(QIODevice* audioSinkDevice){
    if (m_output){
        *m_reader -= *m_output;
    }
    m_audioSinkDevice = audioSinkDevice;
    switch (m_format){
    case AudioFormat::MONO_48000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 1));
        break;
    case AudioFormat::DUAL_44100:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 2));
        break;
    case AudioFormat::DUAL_48000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 2));
        break;
    case AudioFormat::MONO_96000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 2));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 2));
        break;
    case AudioFormat::INTERLEAVE_RL_96000:
        m_output.reset(new AudioSinkWriter(*audioSinkDevice, AudioStreamFormat::FLOAT32, 2));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
    *m_reader += *m_output;
}


qint64 AudioIODevice::writeData(const char* data, qint64 len){
    m_reader->push_bytes(data, len);
    return len;
}






}
