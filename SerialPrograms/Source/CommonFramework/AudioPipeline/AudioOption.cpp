/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EnumStringMap.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Sources/AudioSource_Null.h"
#include "Sources/AudioSource_File.h"
#include "Sources/AudioSource_Device.h"
#include "AudioOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


const std::string AudioOption::JSON_INPUT_FORMAT = "InputFormat";
const std::string AudioOption::JSON_OUTPUT_DEVICE = "OutputDevice";
const std::string AudioOption::JSON_AUDIO_VIS = "AudioVisualization";
const std::string AudioOption::JSON_AUDIO_VOLUME = "Volume";


const EnumStringMap<AudioSourceType> AUDIO_TYPE_STRINGS{
    {AudioSourceType::None,     "InputSource_None"},
    {AudioSourceType::File,     "InputSource_File"},
    {AudioSourceType::Device,   "InputSource_Device"},
};


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

AudioOption::AudioOption()
    : m_input_source(new AudioSourceDescriptor_Null())
{}

void AudioOption::set_input(std::shared_ptr<AudioSourceDescriptor> descriptor){
    m_input_cache[descriptor->type] = descriptor;
    m_input_source = std::move(descriptor);
}
std::shared_ptr<AudioSourceDescriptor> AudioOption::get_input_from_cache(AudioSourceType type){
    auto iter = m_input_cache.find(type);
    if (iter != m_input_cache.end()){
        return iter->second;
    }
//    cout << "Cache Miss: " << (int)type << endl;
    std::shared_ptr<AudioSourceDescriptor> descriptor;
    switch (type){
    case AudioSourceType::None:
        descriptor.reset(new AudioSourceDescriptor_Null());
        break;
    case AudioSourceType::File:
        descriptor.reset(new AudioSourceDescriptor_File());
        break;
    case AudioSourceType::Device:
        descriptor.reset(new AudioSourceDescriptor_Device());
        break;
    default:;
        descriptor.reset(new AudioSourceDescriptor_Null());
    }
    m_input_cache[descriptor->type] = descriptor;
    return descriptor;
}

void AudioOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

//    std::unique_ptr<AudioSourceDescriptor> input_source;

    const std::string* str;

    for (const auto& item : AUDIO_TYPE_STRINGS){
        const JsonValue* params = obj->get_value(AUDIO_TYPE_STRINGS.get_string(item.first));
        if (params == nullptr){
            continue;
        }
        std::shared_ptr<AudioSourceDescriptor> descriptor = get_input_from_cache(item.first);
        descriptor->load_json(*params);
        m_input_cache[descriptor->type] = std::move(descriptor);
    }

    str = obj->get_string("InputSourceType");
    m_input_source = get_input_from_cache(
        str != nullptr
            ? AUDIO_TYPE_STRINGS.get_enum(*str, m_input_source->type)
            : m_input_source->type
    );


    str = obj->get_string(JSON_INPUT_FORMAT);
    if (str != nullptr && !str->empty()){
        for (const AudioInputChoice& choice : m_input_source->supported_formats()){
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

    root["InputSourceType"] = AUDIO_TYPE_STRINGS.get_string(m_input_source->type);

    for (const auto& item : m_input_cache){
        root[AUDIO_TYPE_STRINGS.get_string(item.first)] = item.second->to_json();
    }

    root[JSON_INPUT_FORMAT] = m_input_choice.display_name();
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
