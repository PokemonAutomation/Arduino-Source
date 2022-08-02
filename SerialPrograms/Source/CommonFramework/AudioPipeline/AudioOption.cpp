/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "AudioOption.h"
#include "AudioSelectorWidget.h"

namespace PokemonAutomation{


const std::string AudioSelector::JSON_INPUT_DEVICE = "InputDevice";
const std::string AudioSelector::JSON_INPUT_FORMAT = "InputFormat";
const std::string AudioSelector::JSON_OUTPUT_DEVICE = "OutputDevice";
const std::string AudioSelector::JSON_AUDIO_VIS = "AudioVisualization";
const std::string AudioSelector::JSON_AUDIO_VOLUME = "Volume";

AudioSelector::AudioDisplayType AudioSelector::stringToAudioDisplayType(const std::string& value){
    if (value == "FREQ_BARS"){
        return AudioDisplayType::FREQ_BARS;
    } else if (value == "SPECTROGRAM"){
        return AudioDisplayType::SPECTROGRAM;
    }
    return AudioDisplayType::NO_DISPLAY;
}

std::string AudioSelector::audioDisplayTypeToString(AudioSelector::AudioDisplayType type){
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

AudioSelector::AudioSelector() {}
AudioSelector::AudioSelector(const JsonValue& json){
    load_json(json);
}

void AudioSelector::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* str;
    str = obj->get_string(JSON_INPUT_DEVICE);
    if (str != nullptr){
        m_inputDevice = AudioDeviceInfo(*str);
    }
    str = obj->get_string(JSON_INPUT_FORMAT);
    if (str != nullptr){
        for (AudioFormat format : m_inputDevice.supported_formats()){
            if (AUDIO_FORMAT_LABELS[(size_t)format] == *str){
                m_inputFormat = format;
                break;
            }
        }
    }
//    cout << AUDIO_FORMAT_LABELS[(size_t)m_inputFormat] << endl;
    str = obj->get_string(JSON_OUTPUT_DEVICE);
    if (str != nullptr){
        m_outputDevice = AudioDeviceInfo(*str);
    }
    str = obj->get_string(JSON_AUDIO_VIS);
    if (str != nullptr){
        m_audioDisplayType = stringToAudioDisplayType(*str);
    }
    int volume;
    if (obj->read_integer(volume, JSON_AUDIO_VOLUME, 0, 100)){
        m_volume = volume;
    }
}

JsonValue AudioSelector::to_json() const{
    JsonObject root;
    root[JSON_INPUT_DEVICE] = m_inputDevice.device_name();
    root[JSON_INPUT_FORMAT] = AUDIO_FORMAT_LABELS[(size_t)m_inputFormat];
    root[JSON_OUTPUT_DEVICE] = m_outputDevice.device_name();
    root[JSON_AUDIO_VIS] = audioDisplayTypeToString(m_audioDisplayType);
    root[JSON_AUDIO_VOLUME] = m_volume;
    return root;
}

AudioSelectorWidget* AudioSelector::make_ui(QWidget& parent, LoggerQt& logger, AudioDisplayWidget& holder){
    return new AudioSelectorWidget(parent, logger, *this, holder);
}




}
