/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include <set>
#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "AudioInfo.h"

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

AudioSampleFormat get_sample_format(const QAudioFormat& native_format){
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



AudioFormat::AudioFormat()
    : m_native_format(CONSTRUCT_TOKEN)
{}
AudioFormat::AudioFormat(QAudioFormat native_format)
    : m_native_format(CONSTRUCT_TOKEN, std::move(native_format))
{}

AudioSampleFormat AudioFormat::sample_format() const{
    return get_sample_format(*m_native_format);
}
size_t AudioFormat::sample_rate() const{
    return m_native_format->sampleRate();
}
size_t AudioFormat::channels() const{
    return m_native_format->channelCount();
}

const QAudioFormat& AudioFormat::native_format() const{
    return *m_native_format;
}
QAudioFormat& AudioFormat::native_format(){
    return *m_native_format;
}


namespace{


//  The semantic content of a QAudioFormat for the input pipeline: rate,
//  channel count, channel layout, sample format. (Sample size / type /
//  byte order are derived or irrelevant here.)
bool audio_format_equal(const QAudioFormat& a, const QAudioFormat& b){
    return a.sampleRate() == b.sampleRate()
        && a.channelCount() == b.channelCount()
        && a.channelConfig() == b.channelConfig()
        && a.sampleFormat() == b.sampleFormat();
}

bool audio_format_valid(const QAudioFormat& format){
    return format.sampleRate() > 0
        && format.channelCount() > 0
        && format.sampleFormat() != QAudioFormat::SampleFormat::Unknown;
}


//
//  The standard layout name for a channel count, when the channel config
//  is the standard one for that count (or unknown, in which case the
//  pipeline assumes the standard layout anyway). Mirrors
//  fallback_channel_config() in ChannelMixer.cpp. Returns "" for
//  non-standard layouts and channel counts with no standard layout.
//
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


const char* standard_layout_name(size_t channels, QAudioFormat::ChannelConfig config){
    const QAudioFormat::ChannelConfig standard = standard_channel_config(channels);
    if (standard == QAudioFormat::ChannelConfigUnknown
        || (config != QAudioFormat::ChannelConfigUnknown && config != standard)){
        return "";
    }
    switch (channels){
    case 1: return "Mono";
    case 2: return "Stereo";
    case 3: return "2.1";
    case 4: return "3.1";
    case 5: return "5.0";
    case 6: return "5.1";
    case 7: return "7.0";
    case 8: return "7.1";
    default: return "";
    }
}


}



AudioInputChoice::AudioInputChoice(){}
AudioInputChoice::AudioInputChoice(AudioFormat format)
    : m_format(format)
    , m_physical(format)
{}
AudioInputChoice AudioInputChoice::reinterpreted(
    AudioFormat format, AudioFormat physical, bool swap_channels
){
    AudioInputChoice choice;
    choice.m_format = std::move(format);
    choice.m_physical = std::move(physical);
    choice.m_reinterpreted = true;
    choice.m_swap_channels = swap_channels;
    return choice;
}
bool AudioInputChoice::is_valid() const{
    return audio_format_valid(m_format.native_format());
}
AudioInputChoice::operator bool() const{
    return is_valid();
}
const AudioFormat& AudioInputChoice::format() const{
    return m_format;
}
const AudioFormat& AudioInputChoice::physical_format() const{
    return m_physical;
}
bool AudioInputChoice::is_reinterpreted() const{
    return m_reinterpreted;
}
bool AudioInputChoice::swap_channels() const{
    return m_swap_channels;
}
std::string AudioInputChoice::display_name() const{
    const QAudioFormat& format = m_format.native_format();
    if (!audio_format_valid(format)){
        return "";
    }
    std::string name = std::to_string(format.channelCount()) + " x "
        + tostr_u_commas(format.sampleRate()) + " Hz";
    std::string layout;
    if (m_reinterpreted){
        layout = m_swap_channels ? "Interleaved Stereo R/L" : "Interleaved Stereo L/R";
    }else{
        layout = standard_layout_name((size_t)format.channelCount(), format.channelConfig());
    }
    if (!layout.empty()){
        name += " (" + layout + ")";
    }
    return name;
}


AudioInputChoiceList input_choices(std::vector<AudioFormat> supported, const AudioFormat& preferred){
    AudioInputChoiceList ret;

    //  1. Drop invalid entries.
    std::vector<AudioFormat> valid;
    for (const AudioFormat& format : supported){
        if (audio_format_valid(format.native_format())){
            valid.emplace_back(format);
        }
    }

    //  2. Collapse duplicates by (rate, channels, channel config). When
    //  several sample formats are supported for the same shape, the
    //  preferred sample format wins; otherwise the first seen does.
    std::vector<AudioFormat> deduped;
    for (const AudioFormat& format : valid){
        bool found = false;
        for (AudioFormat& kept : deduped){
            const QAudioFormat& k = kept.native_format();
            const QAudioFormat& f = format.native_format();
            if (k.sampleRate() == f.sampleRate()
                && k.channelCount() == f.channelCount()
                && k.channelConfig() == f.channelConfig()){
                found = true;
                if (audio_format_equal(f, preferred.native_format())){
                    kept = format;
                }
                break;
            }
        }
        if (!found){
            deduped.emplace_back(format);
        }
    }

    //  3. Sort by sample rate, then channel count, then config.
    std::sort(deduped.begin(), deduped.end(), [](const AudioFormat& a, const AudioFormat& b){
        const QAudioFormat& x = a.native_format();
        const QAudioFormat& y = b.native_format();
        if (x.sampleRate() != y.sampleRate()){
            return x.sampleRate() < y.sampleRate();
        }
        if (x.channelCount() != y.channelCount()){
            return x.channelCount() < y.channelCount();
        }
        return x.channelConfig() < y.channelConfig();
    });

    for (const AudioFormat& format : deduped){
        ret.choices.emplace_back(format);
    }

    //  4. The misreporting capture-card case: the only supported format is
    //  1ch x 96000, which is physically two 48 kHz channels interleaved.
    //  Offer the reinterpretations in addition to the native choice.
    if (ret.choices.size() == 1){
        const QAudioFormat& only = ret.choices[0].format().native_format();
        if (only.channelCount() == 1 && only.sampleRate() == 96000){
            const QAudioFormat physical = only;
            QAudioFormat logical;
            //  The channel count must be set before the channel config:
            //  QAudioFormat::setChannelCount() resets the config.
            logical.setSampleRate(48000);
            logical.setSampleFormat(physical.sampleFormat());
            logical.setChannelCount(2);
            logical.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
            ret.choices.emplace_back(
                AudioInputChoice::reinterpreted(AudioFormat(logical), AudioFormat(physical), false)
            );
            ret.choices.emplace_back(
                AudioInputChoice::reinterpreted(AudioFormat(logical), AudioFormat(physical), true)
            );
        }
    }

    //  5. The preferred entry: exact match, otherwise the first.
    ret.preferred_index = 0;
    for (size_t i = 0; i < ret.choices.size(); i++){
        if (audio_format_equal(ret.choices[i].format().native_format(), preferred.native_format())){
            ret.preferred_index = i;
            break;
        }
    }

    return ret;
}



//
//  QAudioDevice (Qt6) does not enumerate full formats, only
//  isFormatSupported() / preferredFormat() / supportedSampleFormats(), so
//  the supported list is probed over the standard grid of sample rates
//  and channel counts. For each (rate, channels) shape the device's
//  sample formats are tried in the order: preferred first, then the rest
//  of supportedSampleFormats() (or common defaults when that is empty),
//  and the first supported one is kept.
//
std::vector<AudioFormat> enumerate_supported_formats(const NativeAudioInfo& info){
    std::vector<AudioFormat> supported;

    const QAudioFormat& preferred = info.preferredFormat();

    std::vector<QAudioFormat::SampleFormat> sample_formats(
        info.supportedSampleFormats().begin(), info.supportedSampleFormats().end()
    );
    if (sample_formats.empty()){
        sample_formats = {
            preferred.sampleFormat(),
            QAudioFormat::SampleFormat::Int16,
            QAudioFormat::SampleFormat::Float,
            QAudioFormat::SampleFormat::Int32,
            QAudioFormat::SampleFormat::UInt8,
        };
    }
    //  The preferred sample format wins ties: move it to the front.
    if (preferred.sampleFormat() != QAudioFormat::SampleFormat::Unknown){
        sample_formats.erase(
            std::remove(
                sample_formats.begin(), sample_formats.end(),
                preferred.sampleFormat()
            ),
            sample_formats.end()
        );
        sample_formats.insert(sample_formats.begin(), preferred.sampleFormat());
    }

    for (int rate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000}){
        for (int channels : {1, 2, 3, 4, 5, 6, 7, 8}){
            for (QAudioFormat::SampleFormat sample_format : sample_formats){
                QAudioFormat candidate;
                candidate.setSampleRate(rate);
                candidate.setSampleFormat(sample_format);
                candidate.setChannelCount(channels);
                candidate.setChannelConfig(standard_channel_config((size_t)channels));
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


AudioInputChoiceList build_input_choices(const NativeAudioInfo& info, const std::string& display_name){
    AudioInputChoiceList ret = input_choices(
        enumerate_supported_formats(info), AudioFormat(info.preferredFormat())
    );

    //  A misreporting card's preferred format is the physical
    //  1ch x 96000; the sensible default is the L/R reinterpretation
    //  (R/L for MiraBox, whose physical channel order is reversed).
    const QAudioFormat& preferred = info.preferredFormat();
    if (ret.choices.size() == 3
        && ret.choices[2].is_reinterpreted()
        && ret.choices[2].swap_channels()
        && preferred.channelCount() == 1 && preferred.sampleRate() == 96000){
        ret.preferred_index = display_name.find("MiraBox") != std::string::npos ? 2 : 1;
    }

    return ret;
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



struct AudioDeviceInfo::Data{
    std::string device_name;    //  For serialization
    std::string display_name;

    std::vector<AudioChannelFormat> supported_formats;
    int preferred_format_index;

    std::vector<AudioInputChoice> supported_choices;
    size_t preferred_choice_index = 0;

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

        {
            AudioInputChoiceList choices =
                build_input_choices(data.info, data.display_name);
            data.supported_choices = std::move(choices.choices);
            data.preferred_choice_index = choices.preferred_index;
        }
    }
#elif QT_VERSION_MAJOR == 6
    for (NativeAudioInfo& device : QMediaDevices::audioInputs()){
        list.emplace_back();
        Data& data = *list.back().m_body;

        data.device_name = device.id().toStdString();
        data.display_name = device.description().toStdString();
        data.info = std::move(device);

        data.supported_formats = supported_input_formats(data.preferred_format_index, data.info, data.display_name);

        {
            AudioInputChoiceList choices =
                build_input_choices(data.info, data.display_name);
            data.supported_choices = std::move(choices.choices);
            data.preferred_choice_index = choices.preferred_index;
        }
    }
#endif

    WallClock end = current_time();
    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
    global_logger_tagged().log("Done querying audio inputs... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);

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

        data.preferred_format_index = -1;

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

        data.preferred_format_index = -1;
    }
#endif

    bool show_all_devices = GlobalSettings::instance().AUDIO_PIPELINE->SHOW_ALL_DEVICES;
    if (show_all_devices){
        return list;
    }

    //  Collapse duplicates: keep the first device per display name. (The
    //  old "most supported formats" ranking is obsolete now that the sink
    //  adapts to the device's native format.)
    std::set<std::string> seen;
    std::vector<AudioDeviceInfo> ret;
    for (AudioDeviceInfo& device : list){
        if (seen.insert(device.display_name()).second){
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
const std::vector<AudioInputChoice>& AudioDeviceInfo::supported_choices() const{
    return m_body->supported_choices;
}
size_t AudioDeviceInfo::preferred_choice_index() const{
    return m_body->preferred_choice_index;
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





















