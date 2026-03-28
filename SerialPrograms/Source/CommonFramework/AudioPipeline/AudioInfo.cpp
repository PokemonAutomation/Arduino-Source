/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "AudioInfo.h"

#ifdef _WIN32
#include <Windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include "CommonFramework/VideoPipeline/Backends/DirectShowCameraList.h"
#endif

//#include <iostream>
//using std::cout;
//using std::endl;


#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#elif QT_VERSION_MAJOR == 6
#include <QAudioDevice>
#include <QMediaDevices>
#else
#error "Unsupported Qt version."
#endif


#if QT_VERSION_MAJOR != 5
#define PA_AUDIO_USE_CHANNEL_CONFIG
#endif


namespace PokemonAutomation{



const char* AUDIO_FORMAT_LABELS[] = {
    "(none)",
    "1 x 48,000 Hz (Mono)",
    "2 x 44,100 Hz (Stereo)",
    "2 x 48,000 Hz (Stereo)",
    "1 x 96,000 Hz (Mono)",
    "1 x 96,000 Hz (Interleaved Stereo L/R)",
    "1 x 96,000 Hz (Interleaved Stereo R/L)",
};

void set_format(QAudioFormat& native_format, AudioChannelFormat format){
    switch (format){
    case AudioChannelFormat::MONO_48000:
#ifdef PA_AUDIO_USE_CHANNEL_CONFIG
        if (native_format.channelConfig() != QAudioFormat::ChannelConfigMono){
            native_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
        }
#else
        if (native_format.channelCount() != 1){
            native_format.setChannelCount(1);
        }
#endif
        if (native_format.sampleRate() != 48000){
            native_format.setSampleRate(48000);
        }
        break;
    case AudioChannelFormat::DUAL_44100:
#ifdef PA_AUDIO_USE_CHANNEL_CONFIG
        if (native_format.channelConfig() != QAudioFormat::ChannelConfigStereo){
            native_format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
        }
#else
        if (native_format.channelCount() != 2){
            native_format.setChannelCount(2);
        }
#endif
        if (native_format.sampleRate() != 44100){
            native_format.setSampleRate(44100);
        }
        break;
    case AudioChannelFormat::DUAL_48000:
#ifdef PA_AUDIO_USE_CHANNEL_CONFIG
        if (native_format.channelConfig() != QAudioFormat::ChannelConfigStereo){
            native_format.setChannelConfig(QAudioFormat::ChannelConfigStereo);
        }
#else
        if (native_format.channelCount() != 2){
            native_format.setChannelCount(2);
        }
#endif
        if (native_format.sampleRate() != 48000){
            native_format.setSampleRate(48000);
        }
        break;
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
#ifdef PA_AUDIO_USE_CHANNEL_CONFIG
        if (native_format.channelConfig() != QAudioFormat::ChannelConfigMono){
            native_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
        }
#else
        if (native_format.channelCount() != 1){
            native_format.setChannelCount(1);
        }
#endif
        if (native_format.sampleRate() != 96000){
            native_format.setSampleRate(96000);
        }
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }
}

AudioSampleFormat get_sample_format(QAudioFormat& native_format){
#if QT_VERSION_MAJOR == 5
    if (native_format.sampleType() == QAudioFormat::SampleType::Float){
        return AudioSampleFormat::FLOAT32;
    }else if (native_format.sampleType() == QAudioFormat::SampleType::UnSignedInt && native_format.sampleSize() == 8){
        return AudioSampleFormat::UINT8;
    }else if (native_format.sampleType() == QAudioFormat::SampleType::SignedInt && native_format.sampleSize() == 16){
        return AudioSampleFormat::SINT16;
    }else if (native_format.sampleType() == QAudioFormat::SampleType::SignedInt && native_format.sampleSize() == 32){
        return AudioSampleFormat::SINT32;
    }else{
        return AudioSampleFormat::INVALID;
    }
#elif QT_VERSION_MAJOR == 6
    switch (native_format.sampleFormat()){
    case QAudioFormat::SampleFormat::Float:
        return AudioSampleFormat::FLOAT32;
    case QAudioFormat::SampleFormat::UInt8:
        return AudioSampleFormat::UINT8;
    case QAudioFormat::SampleFormat::Int16:
        return AudioSampleFormat::SINT16;
    case QAudioFormat::SampleFormat::Int32:
        return AudioSampleFormat::SINT32;
    default:
        return AudioSampleFormat::INVALID;
    }
#else
#error "Unknown Qt version."
#endif
}


std::string format_to_str(const QAudioFormat& format){
    std::string str;
    str += "Preferred Format:\n";
    str += "    Channels: " + std::to_string(format.channelCount()) + "\n";
    str += "    Sample Rate: " + std::to_string(format.sampleRate()) + "\n";
#if QT_VERSION_MAJOR == 5
    str += "    Sample Format: " + std::to_string(format.sampleType()) + "\n";
#else
    str += "    Sample Format: " + std::to_string(format.sampleFormat()) + "\n";
#endif
    return str;
}


//  Return a list of our formats that are supported by this device.
//  "preferred_index" is set to the index of the list that is preferred by the device.
//  If no preferred format matches our formats, -1 is returned.
std::vector<AudioChannelFormat> supported_input_formats(int& preferred_index, const NativeAudioInfo& info, const std::string& display_name){
    QAudioFormat preferred_format = info.preferredFormat();

    std::string str = display_name + "\n";
    str += format_to_str(preferred_format);
    global_logger_tagged().log(str);

//    preferred_format.setSampleSize(16);
//    preferred_format.setSampleType(QAudioFormat::SampleType::SignedInt);

    int preferred_channels = preferred_format.channelCount();
    int preferred_rate = preferred_format.sampleRate();

#if 0
    cout << "display_name = " << display_name << endl;
    cout << "channelCount = " << preferred_format.channelCount() << endl;
    cout << "sampleRate = " << preferred_format.sampleRate() << endl;
    cout << "sampleFormat = " << (int)preferred_format.sampleFormat() << endl;
    cout << "preferred_format = " << info.isFormatSupported(preferred_format) << endl;

    for (const QAudioFormat::SampleFormat& format : info.supportedSampleFormats()){
        cout << "supported format: " << (int)format << endl;
    }
#endif

#if 0
#if QT_VERSION_MAJOR == 6 || (QT_VERSION_MAJOR == 5 && __GNUC__)
    //  On Qt6, "QAudioFormat::preferredFormat()" always returns stereo 44.1 kHz.
    //  Unlike Qt5, it does not return the native format of the device
    //  This actually makes it kind of impossible to figure out what the correct
    //  format we need to use to give proper stereo. So instead, we just assume
    //  mono 96000 which will be the case for standard capture cards.
    preferred_channels = 1;
    preferred_rate = 96000;
#endif
#endif

    std::vector<AudioChannelFormat> ret;
    preferred_index = -1;

//    bool stereo = false;
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::MONO_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::MONO_48000);
        }
    }
#if 1
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::DUAL_44100);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::DUAL_44100);
//            stereo = true;
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::DUAL_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::DUAL_48000);
//            stereo = true;
        }
    }
#endif

    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::INTERLEAVE_LR_96000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
                if (display_name.find("MiraBox") != std::string::npos){
                    preferred_index += 2;
                }else{
                    preferred_index += 1;
                }
            }
            ret.emplace_back(AudioChannelFormat::MONO_96000);
            ret.emplace_back(AudioChannelFormat::INTERLEAVE_LR_96000);
            ret.emplace_back(AudioChannelFormat::INTERLEAVE_RL_96000);
        }
    }

//    cout << "supported formats = " << ret.size() << endl;
    return ret;
}
std::vector<AudioChannelFormat> supported_output_formats(int& preferred_index, const NativeAudioInfo& info){
    QAudioFormat preferred_format = info.preferredFormat();
    int preferred_channels = preferred_format.channelCount();
    int preferred_rate = preferred_format.sampleRate();

    std::vector<AudioChannelFormat> ret;
    preferred_index = -1;

    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::MONO_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::MONO_48000);
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::DUAL_44100);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::DUAL_44100);
        }
    }
    {
        QAudioFormat format = preferred_format;
        set_format(format, AudioChannelFormat::DUAL_48000);
        if (info.isFormatSupported(format)){
            if (format.channelCount() == preferred_channels && format.sampleRate() == preferred_rate){
                preferred_index = (int)ret.size();
            }
            ret.emplace_back(AudioChannelFormat::DUAL_48000);
        }
    }

    return ret;
}




struct AudioDeviceInfo::Data{
    std::string device_name;    //  For serialization
    std::string display_name;

    std::vector<AudioChannelFormat> supported_formats;
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

    WallClock start = current_time();

#if QT_VERSION_MAJOR == 5
    for (NativeAudioInfo& device : QAudioDeviceInfo::availableDevices(QAudio::AudioInput)){
        list.emplace_back();
        Data& data = *list.back().m_body;

        std::string name = device.deviceName().toStdString();
        data.device_name = name;
        data.display_name = std::move(name);
        data.info = std::move(device);

        data.supported_formats = supported_input_formats(data.preferred_format_index, data.info, data.display_name);
//        cout << "data.supported_formats = " << data.supported_formats.size() << endl;
    }
#elif QT_VERSION_MAJOR == 6
    for (NativeAudioInfo& device : QMediaDevices::audioInputs()){
        list.emplace_back();
        Data& data = *list.back().m_body;

        data.device_name = device.id().toStdString();
        data.display_name = device.description().toStdString();
        data.info = std::move(device);

        data.supported_formats = supported_input_formats(data.preferred_format_index, data.info, data.display_name);
    }
#endif

    WallClock end = current_time();
    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
    global_logger_tagged().log("Done querying audio inputs... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);

    //  Log all Qt-enumerated devices for diagnosis.
    global_logger_tagged().log(
        "Qt audio inputs: " + std::to_string(list.size()) + " devices found.",
        COLOR_CYAN
    );
    for (size_t i = 0; i < list.size(); i++){
        global_logger_tagged().log(
            "  Qt audio[" + std::to_string(i) + "]: "
            "display=\"" + list[i].display_name() + "\", "
            "id=\"" + list[i].device_name() + "\", "
            "formats=" + std::to_string(list[i].supported_formats().size()),
            COLOR_CYAN
        );
    }

#ifdef _WIN32
    //  Some capture cards (e.g. AVerMedia GC550) are not visible to Qt6's
    //  audio enumeration.  Enumerate ALL Windows audio capture endpoints
    //  via the Core Audio MMDevice API and DirectShow, then add any that
    //  Qt missed.
    {
        std::vector<std::string> qt_names;
        qt_names.reserve(list.size());
        for (const AudioDeviceInfo& device : list){
            qt_names.push_back(device.display_name());
        }

        //  Collect names of audio capture endpoints Qt doesn't know about.
        std::vector<std::string> missing_names;

        //  --- MMDevice / WASAPI enumeration ---
        {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            bool com_ok = SUCCEEDED(hr);

            IMMDeviceEnumerator* enumerator = nullptr;
            hr = CoCreateInstance(
                __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)
            );
            if (SUCCEEDED(hr) && enumerator){
                IMMDeviceCollection* collection = nullptr;
                //  eCapture + eRender would get everything, but we only need capture.
                //  Also enumerate ALL states to see disabled/unplugged devices.
                hr = enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATEMASK_ALL, &collection);
                if (SUCCEEDED(hr) && collection){
                    UINT count = 0;
                    collection->GetCount(&count);
                    global_logger_tagged().log(
                        "MMDevice audio capture endpoints: " + std::to_string(count) + " found.",
                        COLOR_CYAN
                    );
                    for (UINT i = 0; i < count; i++){
                        IMMDevice* device = nullptr;
                        if (FAILED(collection->Item(i, &device)) || !device) continue;

                        //  Get device state.
                        DWORD state = 0;
                        device->GetState(&state);
                        std::string state_str;
                        switch (state){
                        case DEVICE_STATE_ACTIVE:       state_str = "ACTIVE"; break;
                        case DEVICE_STATE_DISABLED:     state_str = "DISABLED"; break;
                        case DEVICE_STATE_NOTPRESENT:   state_str = "NOT_PRESENT"; break;
                        case DEVICE_STATE_UNPLUGGED:    state_str = "UNPLUGGED"; break;
                        default:                        state_str = "UNKNOWN(" + std::to_string(state) + ")"; break;
                        }

                        IPropertyStore* props = nullptr;
                        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props)) && props){
                            PROPVARIANT var_name;
                            PropVariantInit(&var_name);
                            if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &var_name))){
                                if (var_name.vt == VT_LPWSTR && var_name.pwszVal){
                                    int len = WideCharToMultiByte(CP_UTF8, 0, var_name.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                                    if (len > 0){
                                        std::string name(len - 1, '\0');
                                        WideCharToMultiByte(CP_UTF8, 0, var_name.pwszVal, -1, name.data(), len, nullptr, nullptr);

                                        global_logger_tagged().log(
                                            "  MMDevice[" + std::to_string(i) + "]: "
                                            "\"" + name + "\" state=" + state_str,
                                            COLOR_CYAN
                                        );

                                        //  Only add active devices.
                                        if (state == DEVICE_STATE_ACTIVE){
                                            //  Check if Qt already has this device.
                                            bool found = false;
                                            for (const std::string& qt_name : qt_names){
                                                if (qt_name.find(name) != std::string::npos ||
                                                    name.find(qt_name) != std::string::npos){
                                                    found = true;
                                                    break;
                                                }
                                            }
                                            if (!found){
                                                missing_names.push_back(std::move(name));
                                            }
                                        }
                                    }
                                }
                            }
                            PropVariantClear(&var_name);
                            props->Release();
                        }
                        device->Release();
                    }
                    collection->Release();
                }else{
                    global_logger_tagged().log("MMDevice: EnumAudioEndpoints failed.", COLOR_RED);
                }
                enumerator->Release();
            }else{
                global_logger_tagged().log("MMDevice: CoCreateInstance for IMMDeviceEnumerator failed.", COLOR_RED);
            }
            if (com_ok) CoUninitialize();
        }

        //  --- DirectShow audio capture enumeration ---
        {
            std::vector<std::string> dshow_only = get_directshow_only_audio_devices(qt_names);
            global_logger_tagged().log(
                "DirectShow-only audio devices: " + std::to_string(dshow_only.size()) + " found.",
                COLOR_CYAN
            );
            for (std::string& name : dshow_only){
                global_logger_tagged().log(
                    "  DirectShow audio: \"" + name + "\"",
                    COLOR_CYAN
                );
                //  Avoid duplicates from MMDevice pass.
                bool already_found = false;
                for (const std::string& m : missing_names){
                    if (m.find(name) != std::string::npos ||
                        name.find(m) != std::string::npos){
                        already_found = true;
                        break;
                    }
                }
                if (!already_found){
                    missing_names.push_back(std::move(name));
                }
            }
        }

        //  Add all missing devices to the list.
        global_logger_tagged().log(
            "Audio devices missing from Qt: " + std::to_string(missing_names.size()),
            missing_names.empty() ? COLOR_CYAN : COLOR_ORANGE
        );
        for (const std::string& name : missing_names){
            global_logger_tagged().log(
                "  Adding: \"" + name + "\"",
                COLOR_ORANGE
            );
            list.emplace_back();
            Data& data = *list.back().m_body;
            data.device_name = "dshow_audio:" + name;
            data.display_name = name + " (DirectShow)";
            data.preferred_format_index = -1;
        }

        //  --- Video capture devices with embedded audio pins ---
        //  HDMI capture cards (e.g. AVerMedia GC550) may not register
        //  as standalone audio endpoints.  Their audio is only accessible
        //  through the DirectShow video capture filter's audio pin.
        {
            std::vector<std::string> video_audio = get_video_devices_with_audio_pins();
            global_logger_tagged().log(
                "Video capture devices with audio pins: " + std::to_string(video_audio.size()) + " found.",
                COLOR_CYAN
            );
            for (const std::string& name : video_audio){
                global_logger_tagged().log(
                    "  Video device with audio pin: \"" + name + "\"",
                    COLOR_CYAN
                );

                //  Check if already present (as a Qt device or previously added).
                bool already_present = false;
                for (const std::string& qt_name : qt_names){
                    if (qt_name.find(name) != std::string::npos ||
                        name.find(qt_name) != std::string::npos){
                        already_present = true;
                        break;
                    }
                }
                if (!already_present){
                    for (const AudioDeviceInfo& existing : list){
                        if (existing.display_name().find(name) != std::string::npos ||
                            name.find(existing.display_name()) != std::string::npos){
                            already_present = true;
                            break;
                        }
                    }
                }

                if (!already_present){
                    global_logger_tagged().log(
                        "  Adding HDMI audio: \"" + name + "\"",
                        COLOR_ORANGE
                    );
                    list.emplace_back();
                    Data& data = *list.back().m_body;
                    data.device_name = "dshow_audio:" + name;
                    data.display_name = name + " (HDMI Audio)";
                    data.preferred_format_index = -1;
                }
            }
        }
    }
#endif

    bool show_all_devices = GlobalSettings::instance().AUDIO_PIPELINE->SHOW_ALL_DEVICES;
    if (show_all_devices){
        return list;
    }

    //  Get map of best devices.
    std::map<std::string, size_t> best_devices;
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
        if (current_score >= best_score || show_all_devices){
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
        Data& data = *list.back().m_body;

        std::string name = device.deviceName().toStdString();

        std::string str = name + "\n";
        str += format_to_str(device.preferredFormat());
        global_logger_tagged().log(str);

        data.device_name = name;
        data.display_name = std::move(name);
        data.info = std::move(device);

        data.supported_formats = supported_output_formats(data.preferred_format_index, data.info);

    }
#elif QT_VERSION_MAJOR == 6
    for (NativeAudioInfo& device : QMediaDevices::audioOutputs()){
        list.emplace_back();
        Data& data = *list.back().m_body;

        std::string name = device.description().toStdString();

        std::string str = name + "\n";
        str += format_to_str(device.preferredFormat());
        global_logger_tagged().log(str);

        data.device_name = device.id().toStdString();
        data.display_name = std::move(name);
        data.info = std::move(device);

        data.supported_formats = supported_output_formats(data.preferred_format_index, data.info);
    }
#endif

    bool show_all_devices = GlobalSettings::instance().AUDIO_PIPELINE->SHOW_ALL_DEVICES;
    if (show_all_devices){
        return list;
    }

    //  Get map of greatest format counts.
    std::map<std::string, size_t> most_formats;
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
AudioDeviceInfo::AudioDeviceInfo(const AudioDeviceInfo& x) = default;
void AudioDeviceInfo::operator=(const AudioDeviceInfo& x){
    m_body = x.m_body;
}

AudioDeviceInfo::AudioDeviceInfo()
    : m_body(CONSTRUCT_TOKEN)
{}

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
    m_body.reset();
}

AudioDeviceInfo::operator bool() const{
    return m_body && !m_body->device_name.empty();
}
const std::string& AudioDeviceInfo::display_name() const{
    return m_body->display_name;
}
const std::string& AudioDeviceInfo::device_name() const{
    return m_body->device_name;
}
const std::vector<AudioChannelFormat>& AudioDeviceInfo::supported_formats() const{
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
bool AudioDeviceInfo::operator==(const AudioDeviceInfo& info) const{
    return device_name() == info.device_name();
}





}





















