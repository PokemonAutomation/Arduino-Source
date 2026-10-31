/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioDevice>
#include <QMediaDevices>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/AudioPipeline/AudioInputChoice.h"
#include "CommonFramework/AudioPipeline/Backends/NativeAudioFormats.h"
#include "CommonFramework/Logging/Logger.h"
#include "AudioDeviceInfo.h"

namespace PokemonAutomation{


namespace{


//  A format for the log: its display name and sample format, e.g.
//  "1 x 96,000 Hz (Mono) Int16".
std::string format_to_log_str(const AudioFormat& format){
    std::string str = AudioInputChoice(format).display_name();
    switch (format.native_format().sampleFormat()){
    case QAudioFormat::SampleFormat::UInt8: return str + " UInt8";
    case QAudioFormat::SampleFormat::Int16: return str + " Int16";
    case QAudioFormat::SampleFormat::Int32: return str + " Int32";
    case QAudioFormat::SampleFormat::Float: return str + " Float";
    default: return str;
    }
}


//  One line for the log listing a device's input formats, e.g.
//  "MiraBox Capture: input formats: 1 x 96,000 Hz (Mono) Int16. Current: 1 x 96,000 Hz (Mono) Int16."
std::string native_formats_to_str(
    const std::string& audio_source_display_name,
    const std::vector<AudioFormat>& source_native_formats,
    const AudioFormat& current
){
    std::string str = audio_source_display_name + ": input formats: ";
    for (size_t i = 0; i < source_native_formats.size(); i++){
        str += (i == 0 ? "" : ", ") + format_to_log_str(source_native_formats[i]);
    }
    str += ". Current: " + format_to_log_str(current) + ".";
    return str;
}


//  Build the input format choices for a device.
//  If the device is a Mirabox, set the preferred index to be 1x96k interpreted as interleaved 2x48k R/L.
AudioInputChoiceList build_input_choices(
    const NativeAudioInfo& info,
    const std::string& display_name
){
    // Get native audio formats
    NativeAudioFormats native = native_input_formats(info);
    global_logger_tagged().log(native_formats_to_str(display_name, native.formats, native.current));

    //  Build input choice list:
    //  Note A 1ch x 96000 entry will be appened by its two reinterpreted stereo
    //  choices, "Interleaved Stereo L/R" then "Interleaved Stereo R/L".
    AudioInputChoiceList ret = input_choices(std::move(native.formats), native.current);

    //  A capture card that runs in 1ch x 96000 is really sending two
    //  48 kHz channels interleaved, so the sensible default is a stereo
    //  reading of it: R/L for MiraBox, whose physical channel order is
    //  reversed, and L/R for other cards.
    const QAudioFormat& running = native.current.native_format();
    if (running.channelCount() == 1 && running.sampleRate() == 96000){
        const bool swap = display_name.find("MiraBox") != std::string::npos;
        for (size_t i = 0; i < ret.choices.size(); i++){
            if (ret.choices[i].is_reinterpreted() &&
                ret.choices[i].swap_channels() == swap
            ){
                ret.preferred_index = i;
                break;
            }
        }
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
    global_logger_tagged().log(
        "Done querying audio inputs... " + tostr_fixed(seconds, 3) + " seconds",
        COLOR_CYAN
    );

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
