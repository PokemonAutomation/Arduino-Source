/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <QAudioDevice>
#include <QMediaDevices>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "AudioInfo.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



size_t sample_size(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:
        return sizeof(uint8_t);
    case AudioSampleFormat::SINT16:
        return sizeof(int16_t);
    case AudioSampleFormat::SINT32:
        return sizeof(int32_t);
    case AudioSampleFormat::FLOAT32:
        return sizeof(float);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioSampleFormat: " + std::to_string((size_t)format));
    }
}


Kernels::AudioPipeline::SampleFormat to_sample_format(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:   return Kernels::AudioPipeline::SampleFormat::UINT8;
    case AudioSampleFormat::SINT16:  return Kernels::AudioPipeline::SampleFormat::SINT16;
    case AudioSampleFormat::SINT32:  return Kernels::AudioPipeline::SampleFormat::SINT32;
    case AudioSampleFormat::FLOAT32: return Kernels::AudioPipeline::SampleFormat::FLOAT32;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported audio sample format.");
    }
}


AudioSampleFormat get_sample_format(const QAudioFormat& native_format){
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


//  Sample-format quality ranking used to collapse duplicate shapes:
//  float > int32 > int16 > uint8 (higher wins).
int choice_type_priority(QAudioFormat::SampleFormat sample_format){
    switch (sample_format){
    case QAudioFormat::SampleFormat::Float: return 4;
    case QAudioFormat::SampleFormat::Int32: return 3;
    case QAudioFormat::SampleFormat::Int16: return 2;
    case QAudioFormat::SampleFormat::UInt8: return 1;
    default: return 0;
    }
}


//  Fallback quality score for the choice list, used to pick
//  preferred_index when the device's preferred format is not among the
//  choices: score = type x channel x sqrt(rate), with
//      type:    float 1.0, int32 0.9, int16 0.5, uint8 0.1
//      channel: 2ch 2.0, Nch 2 - 1/N
//      rate:    sqrt(rate)
//  so our usual float stereo 48k / 44.1k shapes rank at the top while
//  higher quality unusual formats still climb.
double choice_score(const QAudioFormat& format){
    double type_score = 0.0;
    switch (format.sampleFormat()){
    case QAudioFormat::SampleFormat::Float: type_score = 1.0; break;
    case QAudioFormat::SampleFormat::Int32: type_score = 0.9; break;
    case QAudioFormat::SampleFormat::Int16: type_score = 0.5; break;
    case QAudioFormat::SampleFormat::UInt8: type_score = 0.1; break;
    default: break;
    }
    const int channels = format.channelCount();
    const double channel_score =
        (channels == 2) ? 2.0 : 2.0 - 1.0 / static_cast<double>(channels);
    return type_score * channel_score * std::sqrt(static_cast<double>(format.sampleRate()));
}


//  The reinterpreted 2ch x 48000 view of a physical 1ch x 96000 stream,
//  in the physical sample format.
AudioInputChoice reinterpreted_96k_choice(
    const AudioFormat& physical_96k_mono, bool swap_channels
){
    const QAudioFormat& physical = physical_96k_mono.native_format();
    QAudioFormat logical;
    //  The channel count must be set before the channel config:
    //  QAudioFormat::setChannelCount() resets the config.
    logical.setSampleRate(48000);
    logical.setSampleFormat(physical.sampleFormat());
    logical.setChannelCount(2);
    logical.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
    return AudioInputChoice::reinterpreted(
        AudioFormat(logical), AudioFormat(physical), swap_channels
    );
}



}



AudioInputChoice::~AudioInputChoice() = default;
AudioInputChoice::AudioInputChoice(const AudioInputChoice& x) = default;
AudioInputChoice& AudioInputChoice::operator=(const AudioInputChoice& x) = default;

AudioInputChoice::AudioInputChoice(){}
AudioInputChoice::AudioInputChoice(AudioFormat format)
    : m_format(format)
    , m_physical(format)
{}
bool AudioInputChoice::operator==(const AudioInputChoice& x) const{
    if (is_valid() != x.is_valid()){
        return false;
    }
    if (!is_valid()){
        return true;
    }
    return is_reinterpreted() == x.is_reinterpreted()
        && swap_channels() == x.swap_channels()
        && m_format.native_format() == x.m_format.native_format()
        && m_physical.native_format() == x.m_physical.native_format();
}
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
    //  highest quality sample format wins: float > int32 > int16 > uint8.
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
                if (choice_type_priority(f.sampleFormat()) > choice_type_priority(k.sampleFormat())){
                    kept = format;
                }
                break;
            }
        }
        if (!found){
            deduped.emplace_back(format);
        }
    }

    //  3. Sort for display: channel count descending, then sample rate
    //  descending, then channel config (deterministic tiebreak).
    std::sort(deduped.begin(), deduped.end(), [](const AudioFormat& a, const AudioFormat& b){
        const QAudioFormat& x = a.native_format();
        const QAudioFormat& y = b.native_format();
        if (x.channelCount() != y.channelCount()){
            return x.channelCount() > y.channelCount();
        }
        if (x.sampleRate() != y.sampleRate()){
            return x.sampleRate() > y.sampleRate();
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
            ret.choices.emplace_back(reinterpreted_96k_choice(ret.choices[0].format(), false));
            ret.choices.emplace_back(reinterpreted_96k_choice(ret.choices[0].format(), true));
        }
    }

    //  5. The preferred entry: the exact match of "preferred"; when there
    //  is no match (preferred invalid or unsupported), the choice with the
    //  highest quality score, ties keeping the earlier index.
    ret.preferred_index = 0;
    bool have_match = false;
    size_t best_index = 0;
    double best_score = -1.0;
    for (size_t i = 0; i < ret.choices.size(); i++){
        if (audio_format_equal(ret.choices[i].format().native_format(), preferred.native_format())){
            ret.preferred_index = i;
            have_match = true;
            break;
        }
        const double score = choice_score(ret.choices[i].format().native_format());
        if (score > best_score){
            best_score = score;
            best_index = i;
        }
    }
    if (!have_match && !ret.choices.empty()){
        ret.preferred_index = best_index;
    }

    return ret;
}




//
//  QAudioDevice (Qt6) does not enumerate full formats, only
//  isFormatSupported() / preferredFormat() / supportedSampleFormats(), so
//  the supported list is probed over the standard grid of sample rates
//  and channel counts. For each (rate, channels) shape the sample
//  formats are tried in fixed quality order (float, int32, int16,
//  uint8) and the first supported one is kept, so our preferred types
//  beat the device's preferred sample format.
//
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


AudioInputChoiceList build_input_choices(
    const NativeAudioInfo& info,
    const std::string& display_name
){
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
    str += "    Sample Format: " + std::to_string(format.sampleFormat()) + "\n";
    return str;
}




struct AudioDeviceInfo::Data{
    std::string device_name;    //  For serialization
    std::string display_name;

    std::vector<AudioInputChoice> supported_choices;
    size_t preferred_choice_index = 0;

    QAudioDevice info;
};
std::vector<AudioDeviceInfo> AudioDeviceInfo::all_input_devices(){
    std::vector<AudioDeviceInfo> list;

    WallClock start = current_time();

    for (NativeAudioInfo& device : QMediaDevices::audioInputs()){
        list.emplace_back();
        Data& data = *list.back().m_body;

        data.device_name = device.id().toStdString();
        data.display_name = device.description().toStdString();
        data.info = std::move(device);

        {
            AudioInputChoiceList choices =
                build_input_choices(data.info, data.display_name);
            data.supported_choices = std::move(choices.choices);
            data.preferred_choice_index = choices.preferred_index;
        }
    }

    WallClock end = current_time();
    double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
    global_logger_tagged().log("Done querying audio inputs... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);

    return list;
}
std::vector<AudioDeviceInfo> AudioDeviceInfo::all_output_devices(){
    std::vector<AudioDeviceInfo> list;

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
    }

    return list;
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
const std::vector<AudioInputChoice>& AudioDeviceInfo::supported_choices() const{
    return m_body->supported_choices;
}
size_t AudioDeviceInfo::preferred_choice_index() const{
    return m_body->preferred_choice_index;
}
const NativeAudioInfo& AudioDeviceInfo::native_info() const{
    return m_body->info;
}
bool AudioDeviceInfo::operator==(const AudioDeviceInfo& info) const{
    return device_name() == info.device_name();
}





}





















