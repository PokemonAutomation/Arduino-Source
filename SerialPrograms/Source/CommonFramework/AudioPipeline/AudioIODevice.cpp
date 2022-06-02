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
AudioIODevice::AudioIODevice(AudioFormat our_format, AudioStreamFormat input_format)
     : QIODevice(nullptr)
     , m_format(our_format)
{
    switch (our_format){
    case AudioFormat::MONO_48000:
        m_reader.reset(new AudioSourceReader(input_format, 1, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 1, false));
        break;
    case AudioFormat::DUAL_44100:
        m_reader.reset(new AudioSourceReader(input_format, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 44100, 2, true));
        break;
    case AudioFormat::DUAL_48000:
        m_reader.reset(new AudioSourceReader(input_format, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::MONO_96000:
        m_reader.reset(new AudioSourceReader(input_format, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_LR_96000:
        m_reader.reset(new AudioSourceReader(input_format, 2, false));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    case AudioFormat::INTERLEAVE_RL_96000:
        m_reader.reset(new AudioSourceReader(input_format, 2, true));
        m_fft_runner.reset(new FFTRunner(*this, 48000, 2, true));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_format));
    }
    *m_reader += *m_fft_runner;
}
void AudioIODevice::setAudioSinkDevice(QIODevice* audioSinkDevice, AudioStreamFormat output_format){
    if (m_output){
        *m_reader -= *m_output;
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
    *m_reader += *m_output;
}




qint64 AudioIODevice::writeData(const char* data, qint64 len){
    m_reader->push_bytes(data, len);
//    m_audioSinkDevice->write(data, len);
    return len;
}






}
