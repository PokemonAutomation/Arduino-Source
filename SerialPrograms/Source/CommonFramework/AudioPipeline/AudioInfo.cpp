/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "AudioInfo.h"

#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
namespace PokemonAutomation{

std::vector<AudioInfoData> get_all_audio_inputs(){
    QList<QAudioDeviceInfo> list = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    std::vector<AudioInfoData> ret;
    for (const QAudioDeviceInfo& info : list){
        QString name = info.deviceName();
        std::string id = name.toStdString();
        ret.emplace_back(std::move(id), std::move(name));
    }
    return ret;
}
std::vector<AudioInfoData> get_all_audio_outputs(){
    QList<QAudioDeviceInfo> list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    std::vector<AudioInfoData> ret;
    for (const QAudioDeviceInfo& info : list){
        QString name = info.deviceName();
        std::string id = name.toStdString();
        ret.emplace_back(std::move(id), std::move(name));
    }
    return ret;
}

}
#elif QT_VERSION_MAJOR == 6
#include <QAudioDevice>
#include <QMediaDevices>
namespace PokemonAutomation{

std::vector<AudioInfoData> get_all_audio_inputs(){
    QList<QAudioDevice> list = QMediaDevices::audioInputs();
    std::vector<AudioInfoData> ret;
    for (const QAudioDevice& info : list){
        QString name = info.description();
        std::string id = info.id().data();
        ret.emplace_back(std::move(id), std::move(name));
    }
    return ret;
}
std::vector<AudioInfoData> get_all_audio_outputs(){
    QList<QAudioDevice> list = QMediaDevices::audioOutputs();
    std::vector<AudioInfoData> ret;
    for (const QAudioDevice& info : list){
        QString name = info.description();
        std::string id = info.id().data();
        ret.emplace_back(std::move(id), std::move(name));
    }
    return ret;
}

}
#else
#error "Unknown Qt Version."
#endif



