/*  Audio Input Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioInfo_H
#define PokemonAutomation_AudioInfo_H

#include <string>

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



}
#endif
