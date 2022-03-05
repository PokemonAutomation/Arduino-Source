/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QtGlobal>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "AudioInfo.h"

#include <iostream>
using std::cout;
using std::endl;


#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#elif QT_VERSION_MAJOR == 6
#include <QAudioDevice>
#include <QMediaDevices>
#else
#error "Unsupported Qt version."
#endif
namespace PokemonAutomation{


const char* AUDIO_FORMAT_LABELS[] = {
    "(none)",
    "1 x 48 kHz (Mono)",
    "2 x 44.1 kHz (Stereo)",
    "2 x 48 kHz (Stereo)",
    "1 x 96 kHz (Mono)",
    "1 x 96 kHz (Stereo iLR)",
    "1 x 96 kHz (Stereo iRL)",
};

void set_format(QAudioFormat& native_format, AudioFormat format){
    switch (format){
    case AudioFormat::MONO_48000:
        native_format.setChannelCount(1);
        native_format.setSampleRate(48000);
        break;
    case AudioFormat::DUAL_44100:
        native_format.setChannelCount(2);
        native_format.setSampleRate(44100);
        break;
    case AudioFormat::DUAL_48000:
        native_format.setChannelCount(2);
        native_format.setSampleRate(48000);
        break;
    case AudioFormat::MONO_96000:
    case AudioFormat::INTERLEAVE_LR_96000:
    case AudioFormat::INTERLEAVE_RL_96000:
        native_format.setChannelCount(1);
        native_format.setSampleRate(96000);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }
#if QT_VERSION_MAJOR == 5
    native_format.setSampleType(QAudioFormat::SampleType::Float);
    native_format.setSampleSize(32);
#elif QT_VERSION_MAJOR == 6
    native_format.setSampleFormat(QAudioFormat::SampleFormat::Float);
#endif
}



//  Return a list of our formats that are supported by this device.
//  "preferred_index" is set to the index of the list that is preferred by the device.
//  If no preferred format matches our formats, -1 is returned.
std::vector<AudioFormat> supported_input_formats(int& preferred_index, const NativeAudioInfo& info, const QString& display_name){
    QAudioFormat preferred_format = info.preferredFormat();
    int preferred_channels = preferred_format.channelCount();
    int preferred_rate = preferred_format.sampleRate();

    std::vector<AudioFormat> ret;
    preferred_index = -1;

    bool stereo = false;
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::MONO_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::MONO_48000);
        }
    }
#if 1
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::DUAL_44100);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::DUAL_44100);
            stereo = true;
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::DUAL_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::DUAL_48000);
            stereo = true;
        }
    }
#endif

    if (stereo){
        return ret;
    }

    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::INTERLEAVE_LR_96000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
                if (display_name.contains("ShadowCast")){
                    preferred_index += 1;
                }else{
                    preferred_index += 2;
                }
            }
            ret.emplace_back(AudioFormat::MONO_96000);
            ret.emplace_back(AudioFormat::INTERLEAVE_LR_96000);
            ret.emplace_back(AudioFormat::INTERLEAVE_RL_96000);
        }
    }

    return ret;
}
std::vector<AudioFormat> supported_output_formats(int& preferred_index, const NativeAudioInfo& info){
    QAudioFormat preferred_format = info.preferredFormat();
    int preferred_channels = preferred_format.channelCount();
    int preferred_rate = preferred_format.sampleRate();

    std::vector<AudioFormat> ret;
    preferred_index = -1;

    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::MONO_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::MONO_48000);
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::DUAL_44100);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::DUAL_44100);
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioFormat::DUAL_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioFormat::DUAL_48000);
        }
    }

    return ret;
}




struct AudioDeviceInfo::Data{
    std::string device_name;    //  For serialization
    QString display_name;

    std::vector<AudioFormat> supported_formats;
    int preferred_format_index;

#if QT_VERSION_MAJOR == 5
    QAudioDeviceInfo info;
#elif QT_VERSION_MAJOR == 6
    QAudioDevice info;
#endif

};
std::vector<AudioDeviceInfo> AudioDeviceInfo::all_input_devices(){
    //  The device list that Qt provides above will include duplicates on Windows
    //  due to Windows having a shadow device (winMM) that seems to be able to
    //  resample to all formats.
    //
    //  https://bugreports.qt.io/browse/QTBUG-75781
    //
    //  What we need to do here it so try to de-dupe the list while keeping the
    //  ones that actually support the formats we need the best.

    //  To do this, for all duplicates we first determine if any of the have a
    //  preferred format that matches a format that we want. If there is, we
    //  immediately throw out all the dupes that don't have a preferred format.
    //  This eliminates all the winMM devices if the primary can be used instead.
    //
    //  Then we throw away all the ones that have fewer supported formats than
    //  the most. Identical (but different) devices will likely have identical
    //  names and format support. They won't be deduped.

    std::vector<AudioDeviceInfo> list;

#if QT_VERSION_MAJOR == 5
    for (NativeAudioInfo& device : QAudioDeviceInfo::availableDevices(QAudio::AudioInput)){
        list.emplace_back();
        Data& data = list.back().m_body;

        QString name = device.deviceName();
        data.device_name = name.toStdString();
        data.display_name = std::move(name);
        data.info = std::move(device);

        data.supported_formats = supported_input_formats(data.preferred_format_index, device, data.display_name);
    }
#elif QT_VERSION_MAJOR == 6
    for (NativeAudioInfo& device : QMediaDevices::audioInputs()){
        list.emplace_back();
        Data& data = list.back().m_body;

        data.device_name = device.id().data();
        data.display_name = device.description();
        data.info = std::move(device);

        data.supported_formats = supported_input_formats(data.preferred_format_index, device, data.display_name);
    }
#endif

    //  Get map of best devices.
    std::map<QString, size_t> best_devices;
    for (const AudioDeviceInfo& device : list){
        size_t& best_score = best_devices[device.display_name()];
        size_t current_score = device.supported_formats().size();
        if (device.preferred_format_index() >= 0){
            current_score += 100;
        }
        best_score = std::max(best_score, current_score);
    }

    //  Keep only devices with the most # of formats. Duplicates allowed.
    std::vector<AudioDeviceInfo> ret;
    for (AudioDeviceInfo& device : list){
        size_t best_score = best_devices.find(device.display_name())->second;
        size_t current_score = device.supported_formats().size();
        if (device.preferred_format_index() >= 0){
            current_score += 100;
        }
        if (current_score >= best_score){
            ret.emplace_back(std::move(device));
        }
    }

    return ret;
}
std::vector<AudioDeviceInfo> AudioDeviceInfo::all_output_devices(){
    //  The device list that Qt provides above will include duplicates on Windows
    //  due to Windows having a shadow device (winMM) that seems to be able to
    //  resample to all formats.
    //
    //  https://bugreports.qt.io/browse/QTBUG-75781
    //
    //  What we need to do here it so try to de-dupe the list while keeping the
    //  ones that actually support the formats we need the best.

    //  To do this, for all duplicates we throw away the ones that have fewer
    //  supported formats than the most. Identical (but different) devices will
    //  likely have identical names and format support. They won't be deduped.
    //
    //  This method will keep the winMM device which we want for audio output.

    std::vector<AudioDeviceInfo> list;

#if QT_VERSION_MAJOR == 5
    for (NativeAudioInfo& device : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)){
        list.emplace_back();
        Data& data = list.back().m_body;

        QString name = device.deviceName();
        data.device_name = name.toStdString();
        data.display_name = std::move(name);
        data.info = std::move(device);

        data.supported_formats = supported_output_formats(data.preferred_format_index, device);
    }
#elif QT_VERSION_MAJOR == 6
    for (NativeAudioInfo& device : QMediaDevices::audioOutputs()){
        list.emplace_back();
        Data& data = list.back().m_body;

        data.device_name = device.id().data();
        data.display_name = device.description();
        data.info = std::move(device);

        data.supported_formats = supported_output_formats(data.preferred_format_index, device);
    }
#endif

    //  Get map of greatest format counts.
    std::map<QString, size_t> most_formats;
    for (AudioDeviceInfo& device : list){
        size_t& count = most_formats[device.display_name()];
        count = std::max(count, device.supported_formats().size());
    }

    //  Keep only devices with the most # of formats. Duplicates allowed.
    std::vector<AudioDeviceInfo> ret;
    for (AudioDeviceInfo& device : list){
        auto iter = most_formats.find(device.display_name());
        if (device.supported_formats().size() >= iter->second){
            ret.emplace_back(std::move(device));
        }
    }

    return ret;
}


AudioDeviceInfo::~AudioDeviceInfo(){}
AudioDeviceInfo::AudioDeviceInfo(const AudioDeviceInfo&) = default;

AudioDeviceInfo::AudioDeviceInfo(){}

AudioDeviceInfo::AudioDeviceInfo(const std::string& device_name){
    for (AudioDeviceInfo& info : all_input_devices()){
        if (device_name == info.device_name()){
            *this = std::move(info);
            return;
        }
    }
    for (AudioDeviceInfo& info : all_output_devices()){
        if (device_name == info.device_name()){
            *this = std::move(info);
            return;
        }
    }
}

AudioDeviceInfo::operator bool() const{
    return !m_body->device_name.empty();
}
const QString& AudioDeviceInfo::display_name() const{
    return m_body->display_name;
}
const std::string& AudioDeviceInfo::device_name() const{
    return m_body->device_name;
}
const std::vector<AudioFormat>& AudioDeviceInfo::supported_formats() const{
    return m_body->supported_formats;
}
int AudioDeviceInfo::preferred_format_index() const{
    return m_body->preferred_format_index;
}
QAudioFormat AudioDeviceInfo::preferred_format() const{
    QAudioFormat format = native_info().preferredFormat();
    int index = preferred_format_index();
    if (index >= 0){
        set_format(format, supported_formats()[index]);
    }
    return format;
}
const NativeAudioInfo& AudioDeviceInfo::native_info() const{
    return m_body->info;
}
bool AudioDeviceInfo::operator==(const AudioDeviceInfo& info){
    return device_name() == info.device_name();
}





}





















