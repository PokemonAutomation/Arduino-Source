/*  Camera Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CameraInfo_H
#define PokemonAutomation_CameraInfo_H

#include <string>

namespace PokemonAutomation{


class CameraInfo{
public:
    CameraInfo() = default;
    explicit CameraInfo(std::string device_name)
        : m_device_name(std::move(device_name))
    {}

    operator bool() const{ return !m_device_name.empty(); }
    const std::string& device_name() const{ return m_device_name; }

    bool operator==(const CameraInfo& info){
        return m_device_name == info.m_device_name;
    }

private:
    std::string m_device_name;
};



}
#endif
