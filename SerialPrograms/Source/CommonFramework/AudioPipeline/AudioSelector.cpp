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

namespace PokemonAutomation{


const QString AudioSelector::JSON_INPUT_DEVICE = "InputDevice";
const QString AudioSelector::JSON_OUTPUT_DEVICE = "OutputDevice";
const QString AudioSelector::JSON_AUDIO_VIS = "AudioVisualization";

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
AudioSelector::AudioSelector(const QJsonValue& json)
{
    load_json(json);
}

void AudioSelector::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QString name;
    if (json_get_string(name, obj, JSON_INPUT_DEVICE)){
        m_inputDevice = AudioInfo(name.toStdString());
    }
    if (json_get_string(name, obj, JSON_OUTPUT_DEVICE)){
        m_outputDevice = AudioInfo(name.toStdString());
    }
    if (json_get_string(name, obj, JSON_AUDIO_VIS)){
        m_audioDisplayType = stringToAudioDisplayType(name.toStdString());
    }
}

QJsonValue AudioSelector::to_json() const{
    QJsonObject root;
    root.insert(JSON_INPUT_DEVICE, QString::fromStdString(m_inputDevice.device_name()));
    root.insert(JSON_OUTPUT_DEVICE, QString::fromStdString(m_outputDevice.device_name()));
    root.insert(JSON_AUDIO_VIS, QString::fromStdString(audioDisplayTypeToString(m_audioDisplayType)));
    return root;
}

AudioSelectorWidget* AudioSelector::make_ui(QWidget& parent, Logger& logger, AudioDisplayWidget& holder){
    return new AudioSelectorWidget(parent, logger, *this, holder);
}




}
