/*  Audio format Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <sstream>
#include <QAudio>
#include <QAudioFormat>
#include "Kernels/AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "AudioFormatUtils.h"



namespace PokemonAutomation{


float convert_audio_volume_from_slider(double volume){
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


std::string dump_audio_format(const QAudioFormat& format){
    std::string sampleFormatStr = "";
    switch(format.sampleFormat()){
        case QAudioFormat::SampleFormat::Float:
            sampleFormatStr="Float";
            break;
        case QAudioFormat::SampleFormat::Int16:
            sampleFormatStr="Int16";
            break;
        case QAudioFormat::SampleFormat::Int32:
            sampleFormatStr="Int32";
            break;
        case QAudioFormat::SampleFormat::UInt8:
            sampleFormatStr="UInt8";
            break;
        default:
            sampleFormatStr="Error";
    }

    std::string channelConfigStr = "";
    switch(format.channelConfig()){
        case QAudioFormat::ChannelConfig::ChannelConfigMono:
            channelConfigStr = "Mono";
            break;
        case QAudioFormat::ChannelConfig::ChannelConfigStereo:
            channelConfigStr = "Stereo";
            break;
        case QAudioFormat::ChannelConfig::ChannelConfigUnknown:
            channelConfigStr = "Unknown";
            break;
        default:
            channelConfigStr = "Non Mono or Stereo";
    }
    std::ostringstream ss;
    ss << "sample format " << sampleFormatStr << 
        ", bytes per sample " << format.bytesPerSample() << 
        ", channel config " << channelConfigStr <<
        ", num channels " << format.channelCount() << 
        ", sample rate " << format.sampleRate() << std::endl;
    return ss.str();
}

void set_sample_format_to_float(QAudioFormat& format){
    format.setSampleFormat(QAudioFormat::SampleFormat::Float);
}

void convert_samples_to_float(const QAudioFormat& format, const char* data, size_t len, float* out){
    switch(format.sampleFormat()){
    case QAudioFormat::SampleFormat::Float:
        memcpy(out, data, len);
        break;
    case QAudioFormat::SampleFormat::Int16:
        Kernels::AudioStreamConverter::convert_from<int16_t>(
            out, reinterpret_cast<const int16_t*>(data), len/sizeof(int16_t), 1.0f
        );
        break;
    case QAudioFormat::SampleFormat::Int32:
        Kernels::AudioStreamConverter::convert_from<int32_t>(
            out, reinterpret_cast<const int32_t*>(data), len/sizeof(int32_t), 1.0f
        );
        break;
    case QAudioFormat::SampleFormat::UInt8:
        Kernels::AudioStreamConverter::convert_from<uint8_t>(
            out, reinterpret_cast<const uint8_t*>(data), len/sizeof(uint8_t), 1.0f
        );
        break;
    default:
        std::cout << "Error: Unkwnon sample format in convert_samples_to_float()" << std::endl;
    }
}




}
