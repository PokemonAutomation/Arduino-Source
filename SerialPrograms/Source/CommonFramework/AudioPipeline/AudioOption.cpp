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
    const std::string* str;
    str = obj->get_string(JSON_INPUT_FILE);
    if (str != nullptr && !str->empty()){
        m_input_source = std::make_shared<AudioSourceDescriptor_File>(*str);
    }else{
        str = obj->get_string(JSON_INPUT_DEVICE);
        if (str != nullptr && !str->empty()){
            AudioDeviceInfo device(*str);
            const std::string* format_str = obj->get_string(JSON_INPUT_FORMAT);
            if (format_str != nullptr && !format_str->empty()){
                for (const AudioInputChoice& choice : device.supported_choices()){
                    if (choice.display_name() == *format_str){
                        m_input_source = std::make_shared<AudioSourceDescriptor_Device>(device, choice);
                        break;
                    }
                }
            }
            if (!m_input_source){
                //  Missing / unknown input format: remember the device with
                //  an invalid choice; AudioSession::load() resolves it to the
                //  preferred choice.
                m_input_source = std::make_shared<AudioSourceDescriptor_Device>(
                    device, AudioInputChoice()
                );
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
    root[JSON_INPUT_FILE] = input_file();
    root[JSON_INPUT_DEVICE] = input_device().device_name();
    root[JSON_INPUT_FORMAT] = input_choice().display_name();
    root[JSON_OUTPUT_DEVICE] = m_output_device.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_display_type);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}





}
