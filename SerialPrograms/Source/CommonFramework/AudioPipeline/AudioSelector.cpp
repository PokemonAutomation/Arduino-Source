/*  Audio Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include "Common/Compiler.h"
#include "Common/Qt/QtJsonTools.h"
#include "AudioSelector.h"
#include "AudioSelectorWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const QString AudioSelector::JSON_INPUT_DEVICE = "InputDevice";
const QString AudioSelector::JSON_INPUT_FORMAT = "InputFormat";
const QString AudioSelector::JSON_OUTPUT_DEVICE = "OutputDevice";
const QString AudioSelector::JSON_AUDIO_VIS = "AudioVisualization";
const QString AudioSelector::JSON_AUDIO_VOLUME = "Volume";

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
AudioSelector::AudioSelector(const QJsonValue& json){
    load_json(json);
}

void AudioSelector::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QString str;
    if (json_get_string(str, obj, JSON_INPUT_DEVICE)){
        m_inputDevice = AudioDeviceInfo(str.toStdString());
    }
    if (json_get_string(str, obj, JSON_INPUT_FORMAT)){
        for (AudioFormat format : m_inputDevice.supported_formats()){
            if (AUDIO_FORMAT_LABELS[(size_t)format] == str){
                m_inputFormat = format;
                break;
            }
        }
    }
//    cout << AUDIO_FORMAT_LABELS[(size_t)m_inputFormat] << endl;
    if (json_get_string(str, obj, JSON_OUTPUT_DEVICE)){
        m_outputDevice = AudioDeviceInfo(str.toStdString());
    }
    if (json_get_string(str, obj, JSON_AUDIO_VIS)){
        m_audioDisplayType = stringToAudioDisplayType(str.toStdString());
    }
    if (json_get_string(str, obj, JSON_AUDIO_VOLUME)){
        m_volume = str.toInt();
    }
}

QJsonValue AudioSelector::to_json() const{
    QJsonObject root;
    root.insert(JSON_INPUT_DEVICE, QString::fromStdString(m_inputDevice.device_name()));
    root.insert(JSON_INPUT_FORMAT, QString::fromStdString(AUDIO_FORMAT_LABELS[(size_t)m_inputFormat]));
    root.insert(JSON_OUTPUT_DEVICE, QString::fromStdString(m_outputDevice.device_name()));
    root.insert(JSON_AUDIO_VIS, QString::fromStdString(audioDisplayTypeToString(m_audioDisplayType)));
    root.insert(JSON_AUDIO_VOLUME, QString::number(m_volume));
    return root;
}

AudioSelectorWidget* AudioSelector::make_ui(QWidget& parent, LoggerQt& logger, AudioDisplayWidget& holder){
    return new AudioSelectorWidget(parent, logger, *this, holder);
}




}
