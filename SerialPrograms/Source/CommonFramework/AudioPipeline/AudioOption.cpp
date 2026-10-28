/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "AudioSources/AudioSource_Null.h"
#include "AudioSources/AudioSource_File.h"
#include "AudioSources/AudioSource_Device.h"
#include "AudioOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


const std::string AudioOption::JSON_INPUT_FILE = "InputFile";
const std::string AudioOption::JSON_INPUT_DEVICE = "InputDevice";
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

AudioOption::AudioOption() = default;

void AudioOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    std::unique_ptr<AudioSourceDescriptor> input_source;

    const std::string* str;
    str = obj->get_string(JSON_INPUT_FILE);
    if (str != nullptr){
        input_source = std::make_unique<AudioSourceDescriptor_File>(*str);
    }
    if (!input_source){
        str = obj->get_string(JSON_INPUT_DEVICE);
        if (str != nullptr){
            input_source = std::make_unique<AudioSourceDescriptor_Device>(*str);
        }
    }
    if (!input_source){
        input_source = std::make_unique<AudioSourceDescriptor_Null>();
    }

    str = obj->get_string(JSON_INPUT_FORMAT);
    if (str != nullptr && !str->empty()){
        for (const AudioInputChoice& choice : input_source->supported_formats()){
            if (choice.display_name() == *str){
                m_input_choice = choice;
                break;
            }
        }
    }

    m_input_source = std::move(input_source);
//    cout << "m_input_choice = " << m_input_choice.display_name() << endl;

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
    AudioSourceType type = AudioSourceType::None;
    if (m_input_source){
        type = m_input_source->type;
    }
    switch (type){
    case AudioSourceType::None:
        break;
    case AudioSourceType::File:
        root[JSON_INPUT_FILE] = static_cast<AudioSourceDescriptor_File&>(*m_input_source).path();
        break;
    case AudioSourceType::Device:
        root[JSON_INPUT_DEVICE] = static_cast<AudioSourceDescriptor_Device&>(*m_input_source).device_info().device_name();
        break;
    }
    root[JSON_INPUT_FORMAT] = m_input_choice.display_name();
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
