/*  Audio format Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include <sstream>
#include <QAudioFormat>
#include "AudioFormatUtils.h"
#include "AudioNormalization.h"



namespace PokemonAutomation{


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
        normalize_audio_le<int16_t>(out, reinterpret_cast<const int16_t*>(data), len/sizeof(int16_t));
        break;
    case QAudioFormat::SampleFormat::Int32:
        normalize_audio_le<int32_t>(out, reinterpret_cast<const int32_t*>(data), len/sizeof(int32_t));
        break;
    case QAudioFormat::SampleFormat::UInt8:
        normalize_audio_le<uint8_t>(out, reinterpret_cast<const uint8_t*>(data), len/sizeof(uint8_t));
        break;
    default:
        std::cout << "Error: Unkwnon sample format in convert_samples_to_float()" << std::endl;
    }
}



float audio_sample_sum(const QAudioFormat& format, const char* data, size_t len){
    const size_t frameBytes = format.bytesPerFrame();
    const size_t numChannels = format.channelCount();
    const size_t sampleBytes = frameBytes / numChannels;
    const size_t numSamples = len / sampleBytes;
    std::vector<float> buffer(numSamples);
    convert_samples_to_float(format, data, len, buffer.data());

    float sum = 0.0f;
    for(float v: buffer){
        sum += v;
    }
    return sum;
}


}
