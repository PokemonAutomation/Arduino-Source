/*  Audio Input Choice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <QAudioFormat>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/AudioPipeline/Backends/NativeAudioFormats.h"
#include "AudioInputChoice.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


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
const char* standard_layout_name(size_t channels, QAudioFormat::ChannelConfig config){
    const QAudioFormat::ChannelConfig standard = standard_channel_config(channels);
    if (standard == QAudioFormat::ChannelConfigUnknown ||
        (config != QAudioFormat::ChannelConfigUnknown && config != standard)
    ){
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
        layout = standard_layout_name(
            (size_t)format.channelCount(),
            format.channelConfig()
        );
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
    //  several sample formats are listed for the same shape, "preferred"
    //  is prioritized. For any other shape the highest quality sample
    //  format wins: float > int32 > int16 > uint8.
    auto rank_audio_format = [](const QAudioFormat& format, const QAudioFormat& preferred) -> int{
        if (audio_format_equal(format, preferred)){
            return std::numeric_limits<int>::max();
        }
        return choice_type_priority(format.sampleFormat());
    };

    std::vector<AudioFormat> deduped;
    for (const AudioFormat& format : valid){
        bool found = false;
        for (AudioFormat& kept : deduped){
            const QAudioFormat& k = kept.native_format();
            const QAudioFormat& f = format.native_format();
            if (k.sampleRate() == f.sampleRate() &&
                k.channelCount() == f.channelCount() &&
                k.channelConfig() == f.channelConfig()
            ){
                found = true;
                if (rank_audio_format(f, preferred.native_format()) > rank_audio_format(k, preferred.native_format())){
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

    //  4. The misreporting capture-card case: 1ch x 96000, which is
    //  physically two 48 kHz channels interleaved.
    //  Offer the reinterpretations in addition to the native choice.
    for (const AudioFormat& format : deduped){
        ret.choices.emplace_back(format);
        const QAudioFormat& only = format.native_format();
        if (only.channelCount() == 1 && only.sampleRate() == 96000){
            ret.choices.emplace_back(reinterpreted_96k_choice(format, false));
            ret.choices.emplace_back(reinterpreted_96k_choice(format, true));
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




}
