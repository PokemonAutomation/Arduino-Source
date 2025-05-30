/*  Camera Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraInfo_H
#define PokemonAutomation_VideoPipeline_CameraInfo_H

#include <string>

namespace PokemonAutomation{


class CameraInfo{
public:
    CameraInfo() = default;
    explicit CameraInfo(std::string device_name)
        : m_device_name(std::move(device_name))
    {}

    void clear(){ m_device_name.clear(); }

    explicit operator bool() const{ return !m_device_name.empty(); }
    const std::string& device_name() const{ return m_device_name; }

    bool operator==(const CameraInfo& info) const{
        return m_device_name == info.m_device_name;
    }

private:
    std::string m_device_name;
};



}
#endif
