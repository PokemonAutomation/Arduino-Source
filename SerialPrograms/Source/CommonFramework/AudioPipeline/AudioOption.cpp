/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "Common/Cpp/EnumStringMap.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "AudioSources/AudioSource_File.h"
#include "AudioSources/AudioSource_Device.h"
#include "AudioSources/AudioSource_Null.h"
#include "AudioOption.h"

namespace PokemonAutomation{


const std::string AudioOption::JSON_SOURCE_TYPE = "SourceType";
const std::string AudioOption::JSON_INPUT_FORMAT = "InputFormat";
const std::string AudioOption::JSON_OUTPUT_DEVICE = "OutputDevice";
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

const EnumStringMap<AudioSourceType> AUDIO_TYPE_STRINGS{
    {AudioSourceType::None,   "None"},
    {AudioSourceType::File,   "Audio File"},
    {AudioSourceType::Device, "Audio Device"},
};


AudioOption::AudioOption()
    : m_input_source(std::make_shared<AudioSourceDescriptor_Null>())
{}
void AudioOption::set_input_source(std::shared_ptr<AudioSourceDescriptor> source){
    if (!source){
        source = std::make_shared<AudioSourceDescriptor_Null>();
    }
    m_input_source = std::move(source);
}

void AudioOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    m_input_source = std::make_shared<AudioSourceDescriptor_Null>();
    const std::string* type = obj->get_string(JSON_SOURCE_TYPE);
    if (type != nullptr){
        const AudioSourceType source_type =
            AUDIO_TYPE_STRINGS.get_enum(*type, AudioSourceType::None);
        const JsonValue* params =
            obj->get_value(AUDIO_TYPE_STRINGS.get_string(source_type));
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

    const std::string* str;
    str = obj->get_string(JSON_INPUT_FORMAT);
    if (str != nullptr && !str->empty()
        && m_input_source->type == AudioSourceType::Device
    ){
        const AudioDeviceInfo& device =
            static_cast<const AudioSourceDescriptor_Device&>(*m_input_source).device_info();
        for (const AudioInputChoice& choice : device.supported_choices()){
            if (choice.display_name() == *str){
                m_input_choice = choice;
                break;
            }
        }
    }
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
        const std::string& type_str = AUDIO_TYPE_STRINGS.get_string(m_input_source->type);
        root[JSON_SOURCE_TYPE] = type_str;
        root[type_str] = m_input_source->to_json();
    }
    root[JSON_INPUT_FORMAT] = m_input_choice.display_name();
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
