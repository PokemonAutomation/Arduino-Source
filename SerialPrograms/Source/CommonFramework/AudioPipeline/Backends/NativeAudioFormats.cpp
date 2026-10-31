/*  Native Audio Formats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioDevice>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "NativeAudioFormats.h"


//  Each platform defines:
//  `NativeAudioFormats query_input_formats(const NativeAudioInfo& device)`
//  as the raw audio formats from OS lib or Qt.
//  `native_input_formats()` calls `query_input_formats()`, then fixes any
//  broken channel layouts from there.
#ifdef __APPLE__
#include "NativeAudioFormats_Apple.tpp"
#else

namespace PokemonAutomation{
namespace{


//  QAudioDevice (Qt6) does not enumerate full formats, only
//  isFormatSupported() / preferredFormat() / supportedSampleFormats(), so
//  the supported list is probed over the standard grid of sample rates
//  and channel counts. For each (rate, channels) shape the sample
//  formats are tried in fixed quality order (float, int32, int16,
//  uint8) and the first supported one is kept, so our preferred types
//  beat the device's preferred sample format.
//
//  The probed formats leave the channel config unknown (Qt's check does
//  not look at it); `native_input_formats()` will fix it later.
//
//  Note: this Qt solution does not work on macOS. Its macOS implementation
//  has some bugs that always returns true to whatever probed format.
std::vector<AudioFormat> enumerate_supported_formats(const NativeAudioInfo& info){
    std::vector<AudioFormat> supported;

    const QAudioFormat& preferred = info.preferredFormat();

    const QAudioFormat::SampleFormat sample_formats[] = {
        QAudioFormat::SampleFormat::Float,
        QAudioFormat::SampleFormat::Int32,
        QAudioFormat::SampleFormat::Int16,
        QAudioFormat::SampleFormat::UInt8,
    };

    for (int rate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000}){
        for (int channels : {1, 2, 3, 4, 5, 6, 7, 8}){
            for (QAudioFormat::SampleFormat sample_format : sample_formats){
                QAudioFormat candidate;
                candidate.setSampleRate(rate);
                candidate.setSampleFormat(sample_format);
                candidate.setChannelCount(channels);
                if (info.isFormatSupported(candidate)){
                    supported.emplace_back(candidate);
                    break;
                }
            }
        }
    }

    if (supported.empty()){
        //  Backends that reject everything (or report no sample formats)
        //  still get an entry so there is always a selection.
        supported.emplace_back(preferred);
    }

    return supported;
}


//  The formats Qt's isFormatSupported() accepts, and the format Qt reports
//  the device running in.
NativeAudioFormats query_input_formats(const NativeAudioInfo& device){
    NativeAudioFormats ret;
    ret.formats = enumerate_supported_formats(device);
    ret.current = AudioFormat(device.preferredFormat());
    return ret;
}


}
}
#endif



namespace PokemonAutomation{



QAudioFormat::ChannelConfig standard_channel_config(size_t channels){
    switch (channels){
    case 1: return QAudioFormat::ChannelConfig::ChannelConfigMono;
    case 2: return QAudioFormat::ChannelConfig::ChannelConfigStereo;
    case 3: return QAudioFormat::ChannelConfig::ChannelConfig2Dot1;
    case 4: return QAudioFormat::ChannelConfig::ChannelConfig3Dot1;
    case 5: return QAudioFormat::ChannelConfig::ChannelConfigSurround5Dot0;
    case 6: return QAudioFormat::ChannelConfig::ChannelConfigSurround5Dot1;
    case 7: return QAudioFormat::ChannelConfig::ChannelConfigSurround7Dot0;
    case 8: return QAudioFormat::ChannelConfig::ChannelConfigSurround7Dot1;
    default: return QAudioFormat::ChannelConfig::ChannelConfigUnknown;
    }
}


namespace{

//  If `format` has unknown channel config but known channel count,
//  use the channel count to fix the channel config.
AudioFormat fix_channel_layout(AudioFormat format){
    QAudioFormat& native = format.native_format();
    const QAudioFormat::ChannelConfig standard = standard_channel_config((size_t)native.channelCount());
    if (native.channelConfig() == QAudioFormat::ChannelConfigUnknown
        && standard != QAudioFormat::ChannelConfigUnknown){
        native.setChannelConfig(standard);
    }
    return format;
}

}


NativeAudioFormats native_input_formats(const NativeAudioInfo& device){
    NativeAudioFormats ret = query_input_formats(device);
    for (AudioFormat& format : ret.formats){
        format = fix_channel_layout(format);
    }
    ret.current = fix_channel_layout(ret.current);
    return ret;
}



}
