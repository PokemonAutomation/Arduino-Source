/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/EnumStringMap.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "AudioOption.h"

namespace PokemonAutomation{


const std::string AudioOption::JSON_AUDIO_SOURCE = "AudioSource";
const std::string AudioOption::JSON_OUTPUT_DEVICE = "OutputDevice";


const EnumStringMap<AudioSourceType> AUDIO_TYPE_STRINGS{
    {AudioSourceType::None,   "None"},
    {AudioSourceType::File,   "Audio File"},
    {AudioSourceType::Device, "Audio Device"},
};
const std::string AudioOption::JSON_AUDIO_VIS = "AudioVisualization";
const std::string AudioOption::JSON_AUDIO_VOLUME = "Volume";

AudioOption::AudioDisplayType AudioOption::stringToAudioDisplayType(const std::string& value){
    if (value == "FREQ_BARS"){
        return AudioDisplayType::FREQ_BARS;
    }else if (value == "SPECTROGRAM"){
        return AudioDisplayType::SPECTROGRAM;
    }
    return AudioDisplayType::NO_DISPLAY;
}

std::string AudioOption::input_file() const{
    if (const std::string* path = m_input_source ? m_input_source->file_path() : nullptr){
        return *path;
    }
    return "";
}
AudioDeviceInfo AudioOption::input_device() const{
    if (const AudioDeviceInfo* device = m_input_source ? m_input_source->input_device() : nullptr){
        return *device;
    }
    return AudioDeviceInfo();
}
AudioInputChoice AudioOption::input_choice() const{
    if (const AudioInputChoice* choice = m_input_source ? m_input_source->input_choice() : nullptr){
        return *choice;
    }
    return AudioInputChoice();
}

std::string AudioOption::audioDisplayTypeToString(AudioOption::AudioDisplayType type){
    switch(type){
    case AudioDisplayType::FREQ_BARS:
        return "FREQ_BARS";
    case AudioDisplayType::SPECTROGRAM:
        return "SPECTROGRAM";
    case AudioDisplayType::NO_DISPLAY:
    default:
        return "NO_DISPLAY";
    }
}

AudioOption::AudioOption(){}

void AudioOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    m_input_source.reset();
    const JsonValue* source = obj->get_value(JSON_AUDIO_SOURCE);
    if (source != nullptr){
        const JsonObject* source_obj = source->to_object();
        if (source_obj != nullptr){
            const std::string* type = source_obj->get_string("SourceType");
            if (type != nullptr){
                const AudioSourceType source_type =
                    AUDIO_TYPE_STRINGS.get_enum(*type, AudioSourceType::None);
                const JsonValue* params =
                    source_obj->get_value(AUDIO_TYPE_STRINGS.get_string(source_type));
                if (params != nullptr){
                    switch (source_type){
                    case AudioSourceType::File:
                    {
                        auto x = std::make_shared<AudioSourceDescriptor_File>();
                        x->load_json(*params);
                        m_input_source = std::move(x);
                        break;
                    }
                    case AudioSourceType::Device:
                    {
                        auto x = std::make_shared<AudioSourceDescriptor_Device>();
                        x->load_json(*params);
                        m_input_source = std::move(x);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
    const std::string* str;
    str = obj->get_string(JSON_OUTPUT_DEVICE);
    if (str != nullptr){
        m_output_device = AudioDeviceInfo(*str);
    }
    str = obj->get_string(JSON_AUDIO_VIS);
    if (str != nullptr){
        m_display_type = stringToAudioDisplayType(*str);
    }
    double volume;
    if (obj->read_float(volume, JSON_AUDIO_VOLUME)){
        volume = std::max(volume, 0.0);
        volume = std::min(volume, 1.0);
        m_volume = volume;
    }
}

JsonValue AudioOption::to_json() const{
    JsonObject root;
    if (m_input_source){
        JsonObject source;
        const std::string& type_str =
            AUDIO_TYPE_STRINGS.get_string(m_input_source->type);
        source["SourceType"] = type_str;
        source[type_str] = m_input_source->to_json();
        root[JSON_AUDIO_SOURCE] = std::move(source);
    }
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
