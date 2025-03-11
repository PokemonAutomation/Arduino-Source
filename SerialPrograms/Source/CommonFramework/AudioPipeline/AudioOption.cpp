/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "AudioOption.h"

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

AudioOption::AudioOption(){}

void AudioOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const std::string* str;
    str = obj->get_string(JSON_INPUT_FILE);
    if (str != nullptr){
        m_input_file = *str;
    }
    if (m_input_file.empty()){
        str = obj->get_string(JSON_INPUT_DEVICE);
        if (str != nullptr){
            m_input_device = AudioDeviceInfo(*str);
        }
    }
    str = obj->get_string(JSON_INPUT_FORMAT);
    if (str != nullptr){
        for (AudioChannelFormat format : m_input_device.supported_formats()){
            if (AUDIO_FORMAT_LABELS[(size_t)format] == *str){
                m_input_format = format;
                break;
            }
        }
    }
//    cout << AUDIO_FORMAT_LABELS[(size_t)m_inputFormat] << endl;
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
    root[JSON_INPUT_FILE] = m_input_file;
    root[JSON_INPUT_DEVICE] = m_input_device.device_name();
    root[JSON_INPUT_FORMAT] = AUDIO_FORMAT_LABELS[(size_t)m_input_format];
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
