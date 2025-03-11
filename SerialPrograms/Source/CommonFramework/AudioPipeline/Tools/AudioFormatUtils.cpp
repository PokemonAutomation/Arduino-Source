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

#if QT_VERSION_MAJOR == 5

std::string dumpAudioFormat(const QAudioFormat& format){
    std::ostringstream ss;
    ss << "Audio format: sample type ";
    switch(format.sampleType()){
        case QAudioFormat::SampleType::Float:
            ss << "Float";
            break;
        case QAudioFormat::SampleType::SignedInt:
            ss << "SignedInt";
            break;
        case QAudioFormat::SampleType::UnSignedInt:
            ss << "UnSignedInt";
            break;
        default:
            ss << "Error";
    }

    ss <<
        ", bytes per sample " << format.bytesPerFrame() / format.channelCount() << 
        ", num channels " << format.channelCount() << 
        ", sample rate " << format.sampleRate() << 
        ", codec " << format.codec().toStdString() << std::endl;

    return ss.str();
}

void setSampleFormatToFloat(QAudioFormat& format){
    format.setSampleType(QAudioFormat::SampleType::Float);
    format.setSampleSize(32);
}

template <typename Type>
void normalize_type(const QAudioFormat& format, const char* data, size_t len, float* out){
    if (format.byteOrder() == QAudioFormat::Endian::LittleEndian){
        normalize_audio_le<Type>(out, reinterpret_cast<const Type*>(data), len/sizeof(Type));
    }else{
        normalize_audio_be<Type>(out, reinterpret_cast<const Type*>(data), len/sizeof(Type));
    }
}

void convertSamplesToFloat(const QAudioFormat& format, const char* data, size_t len, float* out){

    switch(format.sampleType()){
    case QAudioFormat::SampleType::Float:
        if (format.sampleSize() == sizeof(float)*8){
            memcpy(out, data, len);
            return;
        }
        break;
    case QAudioFormat::SampleType::SignedInt:
        switch(format.sampleSize()){
        case 8:
            normalize_type<int8_t>(format, data, len, out);
            return;
        case 16:
            normalize_type<int16_t>(format, data, len, out);
            return;
        case 32:
            normalize_type<int32_t>(format, data, len, out);
            return;
        default:
            break;
        }
        break;
    case QAudioFormat::SampleType::UnSignedInt:
        switch(format.sampleSize()){
        case 8:
            normalize_type<uint8_t>(format, data, len, out);
            return;
        case 16:
            normalize_type<uint16_t>(format, data, len, out);
            return;
        case 32:
            normalize_type<uint32_t>(format, data, len, out);
            return;
        default:
            break;
        }
        break;
    default:
        break;
    }
    std::cout << "Error: Unkwnon sample format in convertSamplesToFloat(): ";
    dumpAudioFormat(format);
}

#elif QT_VERSION_MAJOR == 6

std::string dumpAudioFormat(const QAudioFormat& format){
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

void setSampleFormatToFloat(QAudioFormat& format){
    format.setSampleFormat(QAudioFormat::SampleFormat::Float);
}

void convertSamplesToFloat(const QAudioFormat& format, const char* data, size_t len, float* out){
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
        std::cout << "Error: Unkwnon sample format in convertSamplesToFloat()" << std::endl;
    }
}

#endif


float audioSampleSum(const QAudioFormat& format, const char* data, size_t len){
    const size_t frameBytes = format.bytesPerFrame();
    const size_t numChannels = format.channelCount();
    const size_t sampleBytes = frameBytes / numChannels;
    const size_t numSamples = len / sampleBytes;
    std::vector<float> buffer(numSamples);
    convertSamplesToFloat(format, data, len, buffer.data());

    float sum = 0.0f;
    for(float v: buffer){
        sum += v;
    }
    return sum;
}


}
