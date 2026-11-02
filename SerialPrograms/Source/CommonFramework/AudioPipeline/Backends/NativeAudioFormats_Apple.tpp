/*  Native Audio Formats (Apple)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <string>
#include <vector>
#include <CoreAudio/CoreAudio.h>
#include <QAudioFormat>
#include "NativeAudioFormats.h"

namespace PokemonAutomation{
namespace{


//  The format Qt reports the device running in, as the only format. The
//  fallback when the CoreAudio query fails, e.g. the device was just
//  unplugged.
NativeAudioFormats preferred_format_only(const NativeAudioInfo& device){
    NativeAudioFormats ret;
    ret.current = AudioFormat(device.preferredFormat());
    ret.formats.emplace_back(ret.current);
    return ret;
}


//  Reads a CoreAudio property that holds a single value of "Type".
//  Returns false if the property is missing or the read fails.
template <typename Type>
bool read_property(
    AudioObjectID object,
    AudioObjectPropertySelector selector,
    AudioObjectPropertyScope scope,
    Type& value
){
    const AudioObjectPropertyAddress address{selector, scope, kAudioObjectPropertyElementMain};
    UInt32 bytes = (UInt32)sizeof(Type);
    return AudioObjectGetPropertyData(object, &address, 0, nullptr, &bytes, &value) == noErr
        && bytes == sizeof(Type);
}


//  Reads a CoreAudio property that holds an array of "Type", e.g. the
//  system's device list, a device's streams, or a stream's formats.
//  Returns an empty list if the property is missing or the read fails.
template <typename Type>
std::vector<Type> read_property_list(
    AudioObjectID object,
    AudioObjectPropertySelector selector,
    AudioObjectPropertyScope scope
){
    const AudioObjectPropertyAddress address{selector, scope, kAudioObjectPropertyElementMain};
    UInt32 bytes = 0;
    if (AudioObjectGetPropertyDataSize(object, &address, 0, nullptr, &bytes) != noErr){
        return {};
    }
    std::vector<Type> list(bytes / sizeof(Type));
    if (list.empty()){
        return list;
    }
    bytes = (UInt32)(list.size() * sizeof(Type));
    if (AudioObjectGetPropertyData(object, &address, 0, nullptr, &bytes, list.data()) != noErr){
        return {};
    }
    list.resize(bytes / sizeof(Type));
    return list;
}


//  The device's UID, e.g.
//  "AppleUSBAudioEngine:MACROSILICON:MiraBox Capture:100000:3".
//  Qt uses the UID as QAudioDevice::id() on macOS.
//  Returns "" if it can't be read.
std::string read_device_uid(AudioObjectID device){
    const AudioObjectPropertyAddress address{
        kAudioDevicePropertyDeviceUID,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    CFStringRef uid = nullptr;
    UInt32 bytes = (UInt32)sizeof(uid);
    if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &bytes, &uid) != noErr || uid == nullptr){
        return "";
    }
    char buffer[1024];
    const bool ok = CFStringGetCString(uid, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(uid);
    return ok ? buffer : "";
}


//  The CoreAudio device whose UID is "uid", or kAudioObjectUnknown if no
//  connected device has it.
AudioObjectID find_device(const std::string& uid){
    for (AudioObjectID device : read_property_list<AudioObjectID>(
        kAudioObjectSystemObject, kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal
    )){
        if (read_device_uid(device) == uid){
            return device;
        }
    }
    return kAudioObjectUnknown;
}


//  The Qt sample format that holds a CoreAudio linear PCM format without
//  losing precision: 32-bit float as Float, and integers as UInt8, Int16
//  or Int32. 24-bit integers become Int32: Qt has no 24-bit type, and
//  CoreAudio widens them losslessly. Returns Unknown for anything else,
//  e.g. compressed formats or 64-bit float.
QAudioFormat::SampleFormat to_sample_format(const AudioStreamBasicDescription& format){
    if (format.mFormatID != kAudioFormatLinearPCM){
        return QAudioFormat::SampleFormat::Unknown;
    }
    const UInt32 bits = format.mBitsPerChannel;
    if (format.mFormatFlags & kAudioFormatFlagIsFloat){
        return bits == 32 ? QAudioFormat::SampleFormat::Float : QAudioFormat::SampleFormat::Unknown;
    }
    if (bits == 0){
        return QAudioFormat::SampleFormat::Unknown;
    }
    if (bits <= 8){
        return QAudioFormat::SampleFormat::UInt8;
    }
    if (bits <= 16){
        return QAudioFormat::SampleFormat::Int16;
    }
    if (bits <= 32){
        return QAudioFormat::SampleFormat::Int32;
    }
    return QAudioFormat::SampleFormat::Unknown;
}


//  The Qt format for a CoreAudio stream format at "sample_rate": the
//  sample rate, channel count and sample format are set, the channel
//  config is left unknown. Returns an invalid format if the CoreAudio
//  format has no Qt equivalent.
QAudioFormat to_qt_format(const AudioStreamBasicDescription& format, double sample_rate){
    QAudioFormat ret;
    const QAudioFormat::SampleFormat sample_format = to_sample_format(format);
    if (sample_format == QAudioFormat::SampleFormat::Unknown
        || format.mChannelsPerFrame == 0
        || sample_rate < 1
    ){
        return ret;
    }
    ret.setSampleRate((int)std::lround(sample_rate));
    ret.setChannelCount((int)format.mChannelsPerFrame);
    ret.setSampleFormat(sample_format);
    return ret;
}


//  Adds "format" to "formats" if it is valid and not already there.
//  CoreAudio can list the same format more than once with flags Qt has no
//  use for: the MiraBox lists 16-bit, 1 channel, 96000 Hz twice, once
//  mixable and once non-mixable.
void add_unique_format(std::vector<AudioFormat>& formats, const QAudioFormat& format){
    if (!format.isValid()){
        return;
    }
    for (const AudioFormat& existing : formats){
        if (existing.native_format() == format){
            return;
        }
    }
    formats.emplace_back(format);
}


//  The rates offered when a physical format reports a continuous range of
//  rates instead of a single one.
const double STANDARD_SAMPLE_RATES[] = {
    8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000,
};



//  The device's physical formats from CoreAudio, and the one it runs in
//  right now.
NativeAudioFormats query_input_formats(const NativeAudioInfo& device){
    const AudioObjectID native_device = find_device(device.id().toStdString());
    if (native_device == kAudioObjectUnknown){
        return preferred_format_only(device);
    }

    const std::vector<AudioStreamID> streams = read_property_list<AudioStreamID>(
        native_device, kAudioDevicePropertyStreams, kAudioObjectPropertyScopeInput
    );

    //  Every physical format of every input stream. Each entry is one
    //  hardware format with the range of rates it runs at, which for most
    //  devices is a single rate (min == max).
    NativeAudioFormats ret;
    for (AudioStreamID stream : streams){
        for (const AudioStreamRangedDescription& entry : read_property_list<AudioStreamRangedDescription>(
            stream, kAudioStreamPropertyAvailablePhysicalFormats, kAudioObjectPropertyScopeGlobal
        )){
            const AudioValueRange& rates = entry.mSampleRateRange;
            if (rates.mMaximum - rates.mMinimum < 1){
                add_unique_format(ret.formats, to_qt_format(entry.mFormat, rates.mMinimum));
                continue;
            }
            for (double rate : STANDARD_SAMPLE_RATES){
                if (rates.mMinimum <= rate && rate <= rates.mMaximum){
                    add_unique_format(ret.formats, to_qt_format(entry.mFormat, rate));
                }
            }
        }
    }
    if (ret.formats.empty()){
        return preferred_format_only(device);
    }

    //  The format the device runs in right now: the current physical
    //  format of its first input stream.
    AudioStreamBasicDescription current;
    if (read_property(streams[0], kAudioStreamPropertyPhysicalFormat, kAudioObjectPropertyScopeGlobal, current)){
        ret.current = AudioFormat(to_qt_format(current, current.mSampleRate));
    }
    if (!ret.current.native_format().isValid()){
        ret.current = AudioFormat(device.preferredFormat());
    }

    return ret;
}


}
}
