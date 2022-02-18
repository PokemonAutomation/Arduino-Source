/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInfo_H
#define PokemonAutomation_AudioPipeline_AudioInfo_H

#include <string>
#include <vector>
#include <QString>

namespace PokemonAutomation{


class AudioInfo{
public:
    AudioInfo() = default;
    explicit AudioInfo(std::string device_name)
        : m_device_name(std::move(device_name))
    {}

    operator bool() const{ return !m_device_name.empty(); }

    //  Unique device identifier.
    const std::string& device_name() const{ return m_device_name; }

    bool operator==(const AudioInfo& info){
        return m_device_name == info.m_device_name;
    }

private:
    std::string m_device_name;
};


struct AudioInfoData{
    AudioInfo info;
    QString display_name;

    AudioInfoData(std::string device_name, QString p_display_name)
        : info(std::move(device_name))
        , display_name(std::move(p_display_name))
    {}
};

std::vector<AudioInfoData> get_all_audio_inputs();
std::vector<AudioInfoData> get_all_audio_outputs();




}
#endif
